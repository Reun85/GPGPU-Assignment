#pragma once

#include <imgui.h>

#include "SimulationSettings.h"
struct SimulationData {
  int usedNodes = 0;
  int allocatedNodes = 0;
  float boundingboxstage1ms = 0;
  float boundingboxstage2ms = 0;
  float initOctreems = 0;
  float buildOctreems = 0;
  float centerofMassms = 0;
  float dividecenterofmassms = 0;
  float barneshutms = 0;
  float positionupdatems = 0;
  float deltaTimesecond = 0;
  inline float GetUps() const { return 1.0f / deltaTimesecond; }
  void Render() const {
    if (ImGui::Begin("Simulation Results")) {
      ImGui::Text("Used Nodes: %d, allocated: %d", usedNodes, allocatedNodes);
      ImGui::Text("Bounding Box: %fms, %fms", boundingboxstage1ms,
                  boundingboxstage2ms);
      ImGui::Text("Init Octree: %fms", initOctreems);
      ImGui::Text("Build Octree: %fms", buildOctreems);
      ImGui::Text("Center of Mass: %fms", centerofMassms);
      ImGui::Text("Divide Center of Mass: %fms", dividecenterofmassms);
      ImGui::Text("Barnes-Hut: %fms", barneshutms);
      ImGui::Text("Position Update: %fms", positionupdatems);
      ImGui::Text("Delta Time: %fs, UPS: %f", deltaTimesecond, GetUps());
    }
    ImGui::End();
  }
};

struct SettingChanges {
 public:
  // Only regenerate particles, don't change settings.
  bool only_regenerate = false;
  SimulationSettings new_settings;
};

class Communication {
 public:
  Communication(const SimulationSettings& s) : settings{false, s} {}
  void Handle(SimulationSettingsEditor& sse,
              const SimulationSettingsEditor::Command& cmd) {
    SetRunning(cmd.on);
    if (cmd.apply_changes || cmd.regenerate_particles) {
      std::lock_guard m_change_lock(m_changes);
      std::lock_guard m_settings_lock(m_settings);
      changes = true;
      if (cmd.apply_changes) {
        std::lock_guard m_crash_lock(m_crashed);
        crashed = std::nullopt;
        settings.only_regenerate = false;
        settings.new_settings = sse.GetCurrSettings();
      } else {
        settings.only_regenerate = cmd.regenerate_particles;
      }
    }
  }
  SettingChanges GetNewSettings() {
    std::lock_guard lock(m_settings);
    // Copied
    return settings;
  }

  void SetRunning(bool _new) {
    std::lock_guard lock(m_is_running);
    is_running = _new;
  }
  bool GetRunning() {
    std::lock_guard lock(m_is_running);
    return is_running;
  }
  bool GetRunningOrTrue() {
    bool locked = m_is_running.try_lock();
    bool val = !locked || is_running;
    if (locked) m_is_running.unlock();
    return val;
  }
  void SetShutDown(bool _new) {
    std::lock_guard lock(m_shutdown);
    shutdown = _new;
  }
  bool GetShutDown() {
    std::lock_guard lock(m_shutdown);
    return shutdown;
  }
  bool GetShutDownOrFalse() {
    bool locked = m_shutdown.try_lock();
    bool val = locked && shutdown;
    if (locked) m_shutdown.unlock();
    return val;
  }

  bool GetChangesOrFalseAndReset() {
    bool locked = m_changes.try_lock();
    bool val = locked && changes;
    if (locked) {
      m_changes.unlock();
      changes = false;
    };
    return val;
  }

  SimulationData GetSimulationData() {
    std::lock_guard lock(m_simulationdata);
    return data;
  }
  void SetSimulationData(SimulationData _new) {
    std::lock_guard lock(m_simulationdata);
    data = _new;
  }

  void SetCrashed(CustomCLError ex) {
    std::lock_guard lock(m_crashed);
    crashed = ex;
  }

  std::optional<CustomCLError> GetCrashed() {
    std::lock_guard lock(m_crashed);
    return crashed;
  }

  void RenderSimulationResults() {
    std::lock_guard lock(m_simulationdata);
    data.Render();
  }

  std::mutex m_changes;
  bool changes = false;
  std::mutex m_settings;
  SettingChanges settings;
  std::mutex m_is_running;
  bool is_running = false;
  std::mutex m_shutdown;
  bool shutdown = false;
  std::mutex m_simulationdata;
  SimulationData data;

  std::mutex m_crashed;
  std::optional<CustomCLError> crashed;
};