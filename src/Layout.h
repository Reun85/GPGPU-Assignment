#pragma once

#include <glm/glm.hpp>

#include "NBody.h"

using LayoutResultFunction =
    std::function<ParticleSetDescription(const int size)>;

template <typename T>
class Layout {
  virtual LayoutResultFunction GetResult() const = 0;
  virtual void Render(std::optional<T> prev) = 0;
  virtual bool operator==(const T& other) = 0;
};

class Galaxy : Layout<Galaxy> {
 public:
  static ParticleSetDescription Generate(const int size,
                                         const float default_mass,
                                         const glm::vec3 g_center,
                                         const glm::vec3 g_velocity);
  LayoutResultFunction GetResult() const override;
  void Render(std::optional<Galaxy> prev) override;
  bool operator==(const Galaxy& other) override;

 private:
  glm::vec3 center = glm::vec3(0, 0, 0);
  glm::vec3 velocity = glm::vec3(0, 0, 0);
  float default_mass = 5000;
};

class GalaxiesClashing : Layout<GalaxiesClashing> {
 public:
  static ParticleSetDescription Generate(
      const int size, const float default_mass, const glm::vec3 g1_center,
      const glm::vec3 g2_center, const glm::vec3 g_center,
      const glm::vec3 g1_velocity, const glm::vec3 g2_velocity);
  LayoutResultFunction GetResult() const override;
  void Render(std::optional<GalaxiesClashing> prev) override;
  bool operator==(const GalaxiesClashing& other) override;

 private:
  float default_mass = 5000;
  glm::vec3 galaxy1_center = glm::vec3(3, 3, 3);
  glm::vec3 galaxy2_center = -galaxy2_center;
  glm::vec3 universe_center = glm::vec3(0, 0, 0);
  glm::vec3 galaxy1_velocity = (galaxy2_center - galaxy1_center) * 0.01f;
  glm::vec3 galaxy2_velocity = glm::vec3(0);
};
class Uniform : Layout<Uniform> {
 public:
  static ParticleSetDescription Generate(const int size, const float);
  LayoutResultFunction GetResult() const override;
  void Render(std::optional<Uniform> prev) override;
  bool operator==(const Uniform& other) override;

 private:
  float default_mass = 5000;
};

class LayoutSelector : Layout<LayoutSelector> {
 public:
  enum class SimulationMode { Galaxy, GalaxiesClashing, Uniform };

  static constexpr const char* simulationModeNames[] = {
      "Galaxy", "GalaxiesClashing", "Uniform"};
  LayoutSelector();
  void Render(std::optional<LayoutSelector> prev) override;
  LayoutResultFunction GetResult() const override;
  bool operator==(const LayoutSelector& other) override;

 private:
  std::variant<GalaxiesClashing, Uniform, Galaxy> data_variant;
  SimulationMode simulation_type;
  template <typename T>
  std::optional<T> TryAndParse(std::optional<LayoutSelector>& prev,
                               LayoutSelector& curr);
};
