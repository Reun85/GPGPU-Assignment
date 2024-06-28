#pragma once

#include "SimulationSettings.h"

#include <imgui.h>
struct SimulationData {
  int usedNodes;
  int allocatedNodes;
  float boundingboxstage1ms;
  float boundingboxstage2ms;
  float initOctreems;
  float buildOctreems;
  float centerofMassms;
  float dividecenterofmassms;
  float barneshutms;
  float positionupdatems;
  float deltaTimesecond;
  inline float GetUps() const { return 1.0f / deltaTimesecond; }
  void Render() const {
    if (ImGui::Begin("Simulation Results")) {
      ImGui::Text("Used Nodes: %d, allocated: %d", usedNodes,allocatedNodes);
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
  bool only_restart = true;
  SimulationSettings new_settings;
};

class Communication {
 public:
  void Change(SimulationSettingsEditor& sse,
              SimulationSettingsEditor::State s) {
    SetRunning(false);
    std::lock_guard lock(m_changes);
    changes = true;
    std::lock_guard lock2(m_settings);
    if (s == SimulationSettingsEditor::State::ResetChanges) {
      settings.only_restart = false;
      settings.new_settings = sse.GetCurrSettings();
    } else {
    settings.only_restart = true;
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

  void SetCrashed(std::exception ex) {
    std::lock_guard lock(m_crashed);
    crashed = ex;
  }

  std::optional<std::exception> GetCrashed() {
    std::lock_guard lock(m_crashed);
    return crashed;
  }

  void RenderSimulationResults() {
      std::lock_guard lock(m_simulationdata);
      data.Render();

  }

  std::mutex m_changes;
  bool changes = true;
  std::mutex m_settings;
  SettingChanges settings;
  std::mutex m_is_running;
  bool is_running = false;
  std::mutex m_shutdown;
  bool shutdown = false;
  std::mutex m_simulationdata;
  SimulationData data;

  std::mutex m_crashed;
  std::optional<std::exception> crashed;
};