#include "SimulationSettings.h"

#include <CLPreComp.h>
#include <imgui.h>

#include <sstream>

SimulationSettings::SimulationSettings(const int _particle_count,
                                       const LayoutResultFunction& _layout,
                                       const float _distanceThreshold,
                                       const float _eps,
                                       const float _gravitation_constant,
                                       const int _start_depth)
    : particle_count(_particle_count),
      layout(_layout),
      distanceThreshold(_distanceThreshold),
      eps(_eps),
      gravitational_constant(_gravitation_constant),
      start_depth(_start_depth) {}

bool SimulationSettings::operator==(const SimulationSettings& other) const {
  return particle_count == other.particle_count &&
         distanceThreshold == other.distanceThreshold && eps == other.eps &&
         gravitational_constant == other.gravitational_constant &&
         start_depth == other.start_depth;
}
SimulationSettingsEditor::SimulationSettingsEditor()
    : currlayout(),
      curr(DEFAULT_PARTICLE_COUNT, currlayout.GetResult(), DISTANCE_THRESHOLD,
           EPS, GRAVITATION_CONSTANT, START_DEPTH),
      prev(curr) {}

bool SimulationSettingsEditor::has_anything_changed() const {
  return prev != curr || prevlayout != currlayout;
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

SimulationSettingsEditor::State SimulationSettingsEditor::Render() {
  bool start = false;
  bool stop = false;
  bool reset = false;
  if (ImGui::Begin("Simulation")) {
    ImGui::Text("Changeable at any time:");
    ImGui::Separator();

    ImGui::InputFloat("Barnes-Hut distance threshold", &curr.distanceThreshold,
                      0.005f, 0.02f);
    ImGui::SameLine();
    ShowResetButton(curr.distanceThreshold, prev.distanceThreshold,
                    "threshold");

    ImGui::InputFloat("Barnes-Hut epsilon", &curr.eps, 0.005f, 0.02f, "%e");
    ImGui::SameLine();
    ShowResetButton(curr.eps, prev.eps, "epsilon");

    ImGui::InputFloat("Barnes-Hut gravitational_constant",
                      &curr.gravitational_constant, 0.f, 0.f, "%e");
    ImGui::SameLine();
    ShowResetButton(curr.gravitational_constant, prev.gravitational_constant,
                    "gravitational constant");

    ImGui::InputInt("Octree minimum depth", &curr.start_depth);
    ImGui::SameLine();
    ShowResetButton(curr.start_depth, prev.start_depth, "start depth");

    ImGui::Text("Requires a restart");
    ImGui::Separator();

    ImGui::InputInt("Particle Count", &curr.particle_count);
    ImGui::SameLine();
    ShowResetButton(curr.particle_count, prev.particle_count, "count");

    currlayout.Render(prevlayout);

    ImGui::Separator();
    if (crash.has_value()) {
      std::stringstream s;
      s << "Simulation crashed with: ";
      if (dynamic_cast<cl::Error*>(&crash.value()) != nullptr) {
        cl::Error& error = dynamic_cast<cl::Error&>(crash.value());
        s << error.what() << "(" << error.err()
          << " == " << oclErrorString(error.err()) << ")";
      } else {
        s << crash.value().what();
      }
      ImGui::Text(s.str().c_str());
      ImGui::Separator();
    }

    reset = ImGui::Button("Reset");
    ImGui::SameLine();
    if (!ison) ImGui::BeginDisabled();
    stop = ImGui::Button("Stop");
    if (!ison) ImGui::EndDisabled();
    ImGui::SameLine();
    if (ison) ImGui::BeginDisabled();
    start = ImGui::Button("Start");
    if (ison) ImGui::EndDisabled();
  }
  ImGui::End();

  if (reset) {
    ison = false;
    if (has_anything_changed()) {
      Apply();
      return State::ResetChanges;
    } else
      return State::ResetNoChanges;
  }
  if (start) {
    ison = true;
    return State::On;
  } else if (stop) {
    ison = false;
    return State::Off;
  }

  return State::NoChanges;
}

void SimulationSettingsEditor::Apply() {
  crash = std::nullopt;
  curr.layout = currlayout.GetResult();
  prev = curr;
  prevlayout = currlayout;
}

void SimulationSettingsEditor::SetCrashed(std::exception ex) {
  crash = ex;
  ison = false;
}