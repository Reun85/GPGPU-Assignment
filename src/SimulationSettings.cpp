#include "SimulationSettings.h"

#include <CLPreComp.h>
#include <imgui.h>

#include <functional>
#include <sstream>

bool SimulationSettings::operator==(const SimulationSettings& other) const {
  return particle_count == other.particle_count &&
         distance_threshold == other.distance_threshold && eps == other.eps &&
         gravitational_constant == other.gravitational_constant &&
         start_depth == other.start_depth &&
         barneshut_stack_size == other.barneshut_stack_size &&
         build_octree_stack_size == other.build_octree_stack_size &&
         boundingbox_work_group_size == other.boundingbox_work_group_size &&
         min_enter_depth == other.min_enter_depth &&
         max_depth == other.max_depth &&
         position_update_items_per_thread ==
             other.position_update_items_per_thread &&
         barneshut_items_per_thread == other.barneshut_items_per_thread &&
         divide_by_mass_threads == other.divide_by_mass_threads &&
         center_of_mass_items_per_thread ==
             other.center_of_mass_items_per_thread &&
         allocatedNodes == other.allocatedNodes;
}
SimulationSettingsEditor::SimulationSettingsEditor()
    : currlayout(DEFAULT_LAYOUT),
      curr(DEFAULT_PARTICLE_COUNT, currlayout.GetResult(),
           DEFAULT_DISTANCE_THRESHOLD, DEFAULT_EPS,
           DEFAULT_GRAVITATION_CONSTANT, DEFAULT_START_DEPTH,
           DEFAULT_MIN_ENTER_DEPTH, DEFAULT_MAX_DEPTH,
           DEFAULT_POSITION_UPDATE_ITEMS_PER_THREAD,
           DEFAULT_BARNES_HUT_ITEMS_PER_THREAD, DEFAULT_DIVIDE_BY_MASS_THREADS,
           DEFAULT_CENTER_OF_MASS_ITEMS_PER_THREAD,
           DEFAULT_ALLOCATED_NODES_COUNT, DEFAULT_BOUNDING_BOX_WORK_GROUP_SIZE,
           DEFAULT_MAX_TIME_STEP, DEFAULT_BARNESHUT_STACK_SIZE,
           DEFAULT_BUILD_OCTREE_STACK_SIZE),
      prevlayout(currlayout),
      prev(std::nullopt) {}

#include "ParticleDescription.h"
size_t SimulationSettings::GetVRAMFromSettings() const {
  size_t ret = 0;
  ret += particle_count * sizeof(cl_float4) * (1 + 2);
  ret += particle_count * sizeof(ParticleData);
  ret += boundingbox_work_group_size * sizeof(cl_float3) * 2;
  ret += allocatedNodes * sizeof(Node);
  return ret;
}

bool SimulationSettingsEditor::has_anything_changed() const {
  if (!prev.has_value()) {
    return true;
  }
  return *prev != curr || prevlayout != currlayout;
}
template <typename T>
inline void ShowResetButton(T& curr, T& prev, const char* id) {
  const bool isDisabled = (prev == curr);
  if (isDisabled) {
    ImGui::BeginDisabled();
  }
  const std::string label = std::string("Reset##") + id;
  if (ImGui::Button(label.c_str())) {
    curr = prev;
  }
  if (isDisabled) {
    ImGui::EndDisabled();
  }
}
template <typename T, typename U>
inline void ShowResetButton(T& curr, T& prev, const char* id,
                            std::function<U&(T&)> f) {
  const bool isDisabled = (f(prev) == f(curr));
  if (isDisabled) {
    ImGui::BeginDisabled();
  }
  const std::string label = std::string("Reset##") + id;
  if (ImGui::Button(label.c_str())) {
    f(curr) = f(prev);
  }
  if (isDisabled) {
    ImGui::EndDisabled();
  }
}
template <typename T, typename U>
inline void ShowResetButton(T& curr, std::optional<T>& prev, const char* id,
                            std::function<U&(T&)> f) {
  const bool isDisabled = !prev.has_value() || (f(*prev) == f(curr));
  if (isDisabled) {
    ImGui::BeginDisabled();
  }
  const std::string label = std::string("Reset##") + id;
  if (ImGui::Button(label.c_str())) {
    f(curr) = f(*prev);
  }
  if (isDisabled) {
    ImGui::EndDisabled();
  }
}

std::string ToBestText(size_t bytes) {
  static constexpr int above = 600;
  double b = (double)bytes;
  if (bytes < above) {
    return std::to_string(bytes) + " B";
  }
  if (bytes < 1024 * above) {
    return std::to_string(b / 1024) + " KB";
  }
  if (bytes < 1024 * 1024 * above) {
    return std::to_string(b / (1024 * 1024)) + " MB";
  }
  return std::to_string(b / (1024uLL * 1024uLL * 1024uLL)) + " GB";
}
std::optional<SimulationSettingsEditor::Command>
SimulationSettingsEditor::RenderAndHandleUserInput() {
  bool start = false;
  bool stop = false;
  bool reset = false;
  bool apply = false;
  if (ImGui::Begin("Simulation")) {
    ImGui::Text("Changeable at any time:");
    ImGui::Separator();

    ImGui::InputFloat("Barnes-Hut distance threshold", &curr.distance_threshold,
                      0.005f, 0.02f);
    ImGui::SameLine();
    ShowResetButton<SimulationSettings, float>(
        curr, prev, "threshold",
        [](SimulationSettings& s) -> float& { return s.distance_threshold; });

    ImGui::InputFloat("Barnes-Hut epsilon", &curr.eps, 0.005f, 0.02f, "%e");
    ImGui::SameLine();
    ShowResetButton<SimulationSettings, float>(
        curr, prev, "epsilon",
        [](SimulationSettings& s) -> float& { return s.eps; });

    ImGui::InputFloat("Barnes-Hut gravitational constant",
                      &curr.gravitational_constant, 0.f, 0.f, "%e");
    ImGui::SameLine();
    ShowResetButton<SimulationSettings, float>(
        curr, prev, "gravitational constant",
        [](SimulationSettings& s) -> float& {
          return s.gravitational_constant;
        });

    ImGui::InputInt("Octree minimum depth", &curr.start_depth);
    ImGui::SameLine();
    ShowResetButton<SimulationSettings, int>(
        curr, prev, "start depth",
        [](SimulationSettings& s) -> int& { return s.start_depth; });

    ImGui::Text("Requires a restart");
    ImGui::Separator();

    ImGui::InputInt("Particle Count", &curr.particle_count);
    ImGui::SameLine();
    ShowResetButton<SimulationSettings, int>(
        curr, prev, "particle count",
        [](SimulationSettings& s) -> int& { return s.particle_count; });

    currlayout.RenderAndHandleUserInput(prevlayout);

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Super secret settings")) {
      ImGui::InputInt("Barnes-Hut stack size", &curr.barneshut_stack_size);
      ImGui::SameLine();
      ShowResetButton<SimulationSettings, int>(
          curr, prev, "Barnes-Hut stack size",
          [](SimulationSettings& s) -> int& { return s.barneshut_stack_size; });

      ImGui::InputInt("Build octree stack size", &curr.build_octree_stack_size);
      ImGui::SameLine();
      ShowResetButton<SimulationSettings, int>(
          curr, prev, "Build octree stack size",
          [](SimulationSettings& s) -> int& {
            return s.build_octree_stack_size;
          });

      ImGui::InputInt("Bounding box work group size",
                      &curr.boundingbox_work_group_size);
      ImGui::SameLine();
      ShowResetButton<SimulationSettings, int>(
          curr, prev, "Bounding box work group size",
          [](SimulationSettings& s) -> int& {
            return s.boundingbox_work_group_size;
          });

      ImGui::InputInt("Min enter depth", &curr.min_enter_depth);
      ImGui::SameLine();
      ShowResetButton<SimulationSettings, int>(
          curr, prev, "Min enter depth",
          [](SimulationSettings& s) -> int& { return s.min_enter_depth; });

      ImGui::InputInt("Max depth", &curr.max_depth);
      ImGui::SameLine();
      ShowResetButton<SimulationSettings, int>(
          curr, prev, "Max depth",
          [](SimulationSettings& s) -> int& { return s.max_depth; });

      ImGui::InputInt("Position update items per thread",
                      &curr.position_update_items_per_thread);
      ImGui::SameLine();
      ShowResetButton<SimulationSettings, int>(
          curr, prev, "Position update items per thread",
          [](SimulationSettings& s) -> int& {
            return s.position_update_items_per_thread;
          });

      ImGui::InputInt("Barnes-Hut items per thread",
                      &curr.barneshut_items_per_thread);
      ImGui::SameLine();
      ShowResetButton<SimulationSettings, int>(
          curr, prev, "Barnes-Hut items per thread",
          [](SimulationSettings& s) -> int& {
            return s.barneshut_items_per_thread;
          });

      ImGui::InputInt("Divide by mass threads", &curr.divide_by_mass_threads);
      ImGui::SameLine();
      ShowResetButton<SimulationSettings, int>(
          curr, prev, "Divide by mass threads",
          [](SimulationSettings& s) -> int& {
            return s.divide_by_mass_threads;
          });

      ImGui::InputInt("Center of mass items per thread",
                      &curr.center_of_mass_items_per_thread);
      ImGui::SameLine();
      ShowResetButton<SimulationSettings, int>(
          curr, prev, "Center of mass items per thread",
          [](SimulationSettings& s) -> int& {
            return s.center_of_mass_items_per_thread;
          });

      ImGui::InputInt("Allocated nodes", &curr.allocatedNodes);
      ImGui::SameLine();
      ShowResetButton<SimulationSettings, int>(
          curr, prev, "Allocated nodes",
          [](SimulationSettings& s) -> int& { return s.allocatedNodes; });

      ImGui::InputFloat("Max timestep in seconds", &curr.max_timestep, 0.f, 0.f,
                        "%.3f");
      ImGui::SameLine();
      ShowResetButton<SimulationSettings, float>(
          curr, prev, "Max timestep",
          [](SimulationSettings& s) -> float& { return s.max_timestep; });
    }

    if (crash.has_value()) {
      ImGui::Separator();
      std::stringstream s;
      s << "Simulation crashed with: ";
      const CustomCLError& error = *crash;
      s << error.what() << "(" << error.err()
        << " == " << oclErrorString(error.err()) << ")";
      ImGui::Text("%s", s.str().c_str());
    }
    ImGui::Separator();
    std::stringstream res;
    res << "VRAM Usage: ";
    res << ToBestText(curr.GetVRAMFromSettings()).c_str();
    ImGui::Text(res.str().c_str());

    if (!prev.has_value()) ImGui::BeginDisabled();
    reset = ImGui::Button("Reset");
    if (!prev.has_value()) ImGui::EndDisabled();
    ImGui::SameLine();
    if (!has_anything_changed()) ImGui::BeginDisabled();
    apply = ImGui::Button("Apply");
    if (!has_anything_changed()) ImGui::EndDisabled();
    ImGui::SameLine();
    if (!ison) ImGui::BeginDisabled();
    stop = ImGui::Button("Stop");
    if (!ison) ImGui::EndDisabled();
    ImGui::SameLine();
    if (ison || !prev.has_value()) ImGui::BeginDisabled();
    start = ImGui::Button("Start");
    if (ison || !prev.has_value()) ImGui::EndDisabled();
  }
  ImGui::End();
  Command cmd;

  if (reset) {
    ison = false;
    cmd.regenerate_particles = true;
  }
  if (apply) {
    Apply();
    ison = false;
    cmd.apply_changes = true;
  }
  if (start) {
    ison = true;
  } else if (stop) {
    ison = false;
  }
  cmd.on = ison;
  if (start || stop || apply || reset)
    return cmd;
  else
    return std::nullopt;
}

void SimulationSettingsEditor::Apply() {
  crash = std::nullopt;
  curr.layoutchanged = currlayout != prevlayout;
  curr.layout = currlayout.GetResult();
  prev = curr;
  prevlayout = currlayout;
}

void SimulationSettingsEditor::SetCrashed(std::optional<CustomCLError> ex) {
  crash = ex;
  if (ex.has_value()) {
    ison = false;
    prev = std::nullopt;
  }
}