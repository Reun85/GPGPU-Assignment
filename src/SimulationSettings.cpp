#include "SimulationSettings.h"

#include <imgui.h>

#include "NBody.h"

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

SimulationSettingsEditor::SimulationSettingsEditor()
    : currlayout(),
      curr(PARTICLE_COUNT, currlayout.GetResult(), DISTANCE_THRESHOLD, EPS,
           GRAVITATION_CONSTANT, START_DEPTH),
      prev(curr) {}

template <typename T>
void ShowResetButton(T& curr, T& prev) {
  if (prev == curr) {
    ImGui::BeginDisabled();
  }
  if (ImGui::Button("Reset")) {
    curr = prev;
  }
  if (prev == curr) {
    ImGui::EndDisabled();
  }
}

bool SimulationSettingsEditor::Render() {
  bool apply = false;
  if (ImGui::Begin("Simulation")) {
    ImGui::Text("Changeable at any time:");
    ImGui::Separator();

    ImGui::InputFloat("Barnes-Hut distance threshold", &curr.distanceThreshold,
                      0.005f, 0.02f);
    ImGui::SameLine();
    ShowResetButton(curr.distanceThreshold, prev.distanceThreshold);

    ImGui::InputFloat("Barnes-Hut epsilon", &curr.eps, 0.005f, 0.02f);
    ImGui::SameLine();
    ShowResetButton(curr.eps, prev.eps);

    ImGui::InputFloat("Barnes-Hut gravitational_constant",
                      &curr.gravitational_constant);
    ImGui::SameLine();
    ShowResetButton(curr.gravitational_constant, prev.gravitational_constant);

    ImGui::InputFloat("Octree minimum depth", &curr.gravitational_constant);
    ImGui::SameLine();
    ShowResetButton(curr.start_depth, prev.start_depth);

    ImGui::Text("Requires a restart");
    ImGui::Separator();

    ImGui::InputInt("Particle Count", &curr.particle_count);
    ImGui::SameLine();
    ShowResetButton(curr.particle_count, prev.particle_count);

    currlayout.Render(prevlayout);

    ImGui::Separator();
    apply = ImGui::Button("Restart");
  }
  ImGui::End();

  return apply && has_anything_changed();
}

void SimulationSettingsEditor::Save() {
  prev = curr;
  prevlayout = currlayout;
}
