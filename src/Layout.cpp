#include "Layout.h"

#include <imgui.h>

#include <exception>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optional>
#include <random>
#include <variant> ParticleSetDescription Uniform::Generate(const int size,
                                         const float default_mass) {
                                           std::vector<cl_float4> particles;
                                           std::vector<ParticleData>
                                               particle_data;
                                           cl_float3 empty;
                                           empty.x = 0;
                                           empty.y = 0;
                                           empty.z = 0;
                                           particles.reserve(size);
                                           particle_data.reserve(size);

                                           std::default_random_engine generator;
                                           std::normal_distribution<float>
                                               distribution(0.0f, 1.0);

                                           for (size_t i = 0; i < size; ++i) {
                                             float x = distribution(generator);
                                             float y = distribution(generator);
                                             float z = distribution(generator);
                                             cl_float4 t;
                                             t.x = x;
                                             t.y = y;
                                             t.z = z;
                                             t.w = default_mass;
                                             x = distribution(generator);
                                             y = distribution(generator);
                                             z = distribution(generator);
                                             t.x = x;
                                             t.y = y;
                                             t.z = z;

                                             particles.push_back(t);
                                             particle_data.push_back(
                                                 ParticleData{t, empty});
                                           }

                                           return std::make_pair(particles,
                                                                 particle_data);
                                         }

                                         LayoutResultFunction
                                         Uniform::GetResult() const {
                                           float mass = default_mass;
                                           return [mass](const int size) {
                                             return Uniform::Generate(size,
                                                                      mass);
                                           };
                                         }

                                         const float PI = 3.14159265359;

                                         void GalaxySinglePointGen(
                                             std::normal_distribution<float>
                                                 &distribution,
                                             std::default_random_engine
                                                 &generator,
                                             const float &default_mass, float r,
                                             const glm::vec3 &g2_center,
                                             const glm::vec3 &g2_velocity,
                                             std::vector<cl_float4> &particles,
                                             std::vector<ParticleData>
                                                 &particle_data) {
                                           float rng1 =
                                               distribution(generator) * 2.f *
                                               PI;
                                           float rng2 = distribution(generator);
                                           float rng3 = distribution(generator);
                                           float rng4 = distribution(generator);
                                           rng4 = (pow((rng4 + 1.0f) / 2.0f,
                                                       1.0f) +
                                                   0.25f) *
                                                  default_mass;
                                           glm::vec3 _pos = glm::vec3(
                                               cos(rng1) * rng2 * 1.0f,
                                               rng3 / 20.0f,
                                               sin(rng1) * rng2 * 1.0f);
                                           glm::mat4 rotationMatrix =
                                               glm::rotate(
                                                   glm::mat4(1.0f), r,
                                                   glm::vec3(1.0f, 0.0f, 0.0f));
                                           _pos = glm::vec3(
                                                      rotationMatrix *
                                                      glm::vec4(_pos, 1.0f)) +
                                                  g2_center;

                                           glm::vec3 tang_vel = glm::vec3(
                                               glm::normalize(glm::cross(
                                                   glm::vec3(0, 1, 0),
                                                   glm::vec3(_pos) -
                                                       g2_center)));
                                           float dis = glm::distance(
                                               glm::vec3(_pos), g2_center);
                                           glm::vec3 rnd_vel =
                                               tang_vel * (dis) * 25.f;
                                           rnd_vel /= 100;

                                           rnd_vel += g2_velocity;

                                           particles.push_back(
                                               {{_pos.x, _pos.y, _pos.z,
                                                 rng4}});
                                           particle_data.push_back(
                                               {{{rnd_vel.x, rnd_vel.y,
                                                  rnd_vel.z}},
                                                {{0, 0, 0}}});
                                         }

                                         ParticleSetDescription
                                         GalaxiesClashing::Generate(
                                             const int size,
                                             const float default_mass,
                                             const glm::vec3 g1_center,
                                             const glm::vec3 g2_center,
                                             const glm::vec3 g_center,
                                             const glm::vec3 g1_velocity,
                                             const glm::vec3 g2_velocity) {
                                           std::vector<cl_float3> particles;
                                           std::vector<ParticleData>
                                               particle_data;
                                           particles.reserve(size);
                                           particle_data.reserve(size);

                                           std::default_random_engine generator;
                                           std::normal_distribution<float>
                                               distribution(0.0, 1.0);

                                           float r =
                                               distribution(generator) * PI;
                                           for (size_t i = 0; i < size / 2;
                                                ++i) {
                                             GalaxySinglePointGen(
                                                 distribution, generator,
                                                 default_mass, r, g1_center,
                                                 g1_velocity, particles,
                                                 particle_data);
                                           }
                                           r = distribution(generator) * PI;
                                           for (size_t i = 0;
                                                i < size - size / 2; ++i) {
                                             GalaxySinglePointGen(
                                                 distribution, generator,
                                                 default_mass, r, g2_center,
                                                 g2_velocity, particles,
                                                 particle_data);
                                           }
                                           return std::make_pair(particles,
                                                                 particle_data);
                                         }

                                         ParticleSetDescription
                                         Galaxy::Generate(
                                             const int size,
                                             const float default_mass,
                                             const glm::vec3 g_center,
                                             const glm::vec3 g_velocity) {
                                           std::vector<cl_float3> particles;
                                           std::vector<ParticleData>
                                               particle_data;
                                           particles.reserve(size);
                                           particle_data.reserve(size);

                                           std::default_random_engine generator;
                                           std::normal_distribution<float>
                                               distribution(0.0, 1.0);

                                           float r =
                                               distribution(generator) * PI;
                                           for (size_t i = 0; i < size; ++i) {
                                             GalaxySinglePointGen(
                                                 distribution, generator,
                                                 default_mass, r, g_center,
                                                 g_velocity, particles,
                                                 particle_data);
                                           }
                                           return std::make_pair(particles,
                                                                 particle_data);
                                         }

                                         // GetResults

                                         LayoutResultFunction
                                         Galaxy::GetResult() const {
                                           float mass = default_mass;
                                           glm::vec3 g_center = center;
                                           glm::vec3 g_velocity = velocity;
                                           return [mass, g_center,
                                                   g_velocity](const int size) {
                                             return Galaxy::Generate(
                                                 size, mass, g_center,
                                                 g_velocity);
                                           };
                                         }

                                         LayoutResultFunction
                                         GalaxiesClashing::GetResult() const {
                                           float mass = default_mass;

                                           glm::vec3 g1_center = galaxy1_center;
                                           glm::vec3 g2_center = galaxy2_center;
                                           glm::vec3 g_center = universe_center;
                                           glm::vec3 g1_velocity =
                                               galaxy1_velocity;
                                           glm::vec3 g2_velocity =
                                               galaxy2_velocity;
                                           return [mass, g1_center, g2_center,
                                                   g_center, g1_velocity,
                                                   g2_velocity](
                                                      const int size) {
                                             return GalaxiesClashing::Generate(
                                                 size, mass, g1_center,
                                                 g2_center, g_center,
                                                 g1_velocity, g2_velocity);
                                           };
                                         }
                                         void Galaxy::Render(
                                             std::optional<Galaxy> prev) {
                                           ImGui::InputFloat3(
                                               "Galaxy center",
                                               glm::value_ptr(center));

                                           bool show = prev.has_value() &&
                                                       prev->center != center;
                                           ImGui::SameLine();
                                           if (!show) {
                                             ImGui::BeginDisabled();
                                           }
                                           if (ImGui::Button("Reset")) {
                                             center = prev->center;
                                           }
                                           if (!show) {
                                             ImGui::EndDisabled();
                                           }

                                           ImGui ::InputFloat3(
                                               "Galaxy velocity",
                                               glm::value_ptr(velocity));
                                           show = prev.has_value() &&
                                                  prev->velocity != velocity;
                                           ImGui::SameLine();
                                           if (!show) {
                                             ImGui::BeginDisabled();
                                           }
                                           if (ImGui::Button("Reset")) {
                                             velocity = prev->velocity;
                                           }
                                           if (!show) {
                                             ImGui::EndDisabled();
                                           }

                                           ImGui::InputFloat(
                                               "Average mass of particles",
                                               &default_mass, 10.f, 100.f);
                                           show = prev.has_value() &&
                                                  prev->default_mass !=
                                                      default_mass;
                                           ImGui::SameLine();
                                           if (!show) {
                                             ImGui::BeginDisabled();
                                           }
                                           if (ImGui::Button("Reset")) {
                                             default_mass = prev->default_mass;
                                           }
                                           if (!show) {
                                             ImGui::EndDisabled();
                                           }
                                         }

                                         void GalaxiesClashing::Render(
                                             std::optional<GalaxiesClashing>
                                                 prev) {
                                           ImGui::InputFloat3(
                                               "Galaxy 1 Center",
                                               glm::value_ptr(galaxy1_center));

                                           bool show = prev.has_value() &&
                                                       prev->galaxy1_center !=
                                                           galaxy1_center;
                                           ImGui::SameLine();
                                           if (!show) {
                                             ImGui::BeginDisabled();
                                           }
                                           if (ImGui::Button("Reset")) {
                                             galaxy1_center =
                                                 prev->galaxy1_center;
                                           }
                                           if (!show) {
                                             ImGui::EndDisabled();
                                           }

                                           ImGui::InputFloat3(
                                               "Galaxy 2 Center",
                                               glm::value_ptr(galaxy2_center));
                                           show = prev.has_value() &&
                                                  prev->galaxy2_center !=
                                                      galaxy2_center;
                                           ImGui::SameLine();
                                           if (!show) {
                                             ImGui::BeginDisabled();
                                           }
                                           if (ImGui::Button("Reset")) {
                                             galaxy2_center =
                                                 prev->galaxy2_center;
                                           }
                                           if (!show) {
                                             ImGui::EndDisabled();
                                           }

                                           ImGui::InputFloat3(
                                               "Universe Center",
                                               glm::value_ptr(universe_center));
                                           show = prev.has_value() &&
                                                  prev->universe_center !=
                                                      universe_center;
                                           ImGui::SameLine();
                                           if (!show) {
                                             ImGui::BeginDisabled();
                                           }
                                           if (ImGui::Button("Reset")) {
                                             universe_center =
                                                 prev->universe_center;
                                           }
                                           if (!show) {
                                             ImGui::EndDisabled();
                                           }

                                           ImGui::InputFloat3(
                                               "Galaxy 1 Velocity",
                                               glm::value_ptr(
                                                   galaxy1_velocity));
                                           show = prev.has_value() &&
                                                  prev->galaxy1_velocity !=
                                                      galaxy1_velocity;
                                           ImGui::SameLine();
                                           if (!show) {
                                             ImGui::BeginDisabled();
                                           }
                                           if (ImGui::Button("Reset")) {
                                             galaxy1_velocity =
                                                 prev->galaxy1_velocity;
                                           }
                                           if (!show) {
                                             ImGui::EndDisabled();
                                           }

                                           ImGui::InputFloat3(
                                               "Galaxy 2 Velocity",
                                               glm::value_ptr(
                                                   galaxy2_velocity));
                                           show = prev.has_value() &&
                                                  prev->galaxy2_velocity !=
                                                      galaxy2_velocity;
                                           ImGui::SameLine();
                                           if (!show) {
                                             ImGui::BeginDisabled();
                                           }
                                           if (ImGui::Button("Reset")) {
                                             galaxy2_velocity =
                                                 prev->galaxy2_velocity;
                                           }
                                           if (!show) {
                                             ImGui::EndDisabled();
                                           }

                                           ImGui::InputFloat(
                                               "Average mass of particles",
                                               &default_mass, 10.f, 100.f);
                                           show = prev.has_value() &&
                                                  prev->default_mass !=
                                                      default_mass;
                                           ImGui::SameLine();
                                           if (!show) {
                                             ImGui::BeginDisabled();
                                           }
                                           if (ImGui::Button("Reset")) {
                                             default_mass = prev->default_mass;
                                           }
                                           if (!show) {
                                             ImGui::EndDisabled();
                                           }
                                         }
                                         void Uniform::Render(
                                             std::optional<Uniform> prev) {
                                           ImGui::InputFloat(
                                               "Average mass of particles",
                                               &default_mass, 10.f, 100.f);

                                           bool show = prev.has_value() &&
                                                       prev->default_mass !=
                                                           default_mass;
                                           ImGui::SameLine();
                                           if (!show) {
                                             ImGui::BeginDisabled();
                                           }
                                           if (ImGui::Button("Reset")) {
                                             default_mass = prev->default_mass;
                                           }
                                           if (!show) {
                                             ImGui::EndDisabled();
                                           }
                                         }
                                         LayoutSelector::LayoutSelector() {
                                           data_variant = Galaxy();
                                           simulation_type =
                                               SimulationMode::Galaxy;
                                         }

                                         template <typename T>
                                         std::optional<T>
                                         LayoutSelector::TryAndParse(
                                             std::optional<LayoutSelector>
                                                 &prev,

                                             LayoutSelector &curr) {
                                           if (prev.has_value() &&
                                               prev->simulation_type !=
                                                   curr.simulation_type &&
                                               std::holds_alternative<T>(
                                                   prev->data_variant)) {
                                             return std::optional(std::get<T>(
                                                 prev->data_variant));
                                           } else {
                                             return std::nullopt;
                                           }
                                         }

                                         void LayoutSelector::Render(
                                             std::optional<LayoutSelector>
                                                 prev) {
                                           // Convert current mode to an integer
                                           // index for ImGui::Combo
                                           int currentItem = static_cast<int>(
                                               simulation_type);

                                           // Create the dropdown menu
                                           if (ImGui::Combo(
                                                   "Simulation Mode",
                                                   &currentItem,
                                                   simulationModeNames,
                                                   IM_ARRAYSIZE(
                                                       simulationModeNames))) {
                                             // Update current mode based on
                                             // selection
                                             simulation_type =
                                                 static_cast<SimulationMode>(
                                                     currentItem);
                                             switch (simulation_type) {
                                               case SimulationMode::Galaxy:
                                                 std::get<Galaxy>(data_variant)
                                                     .Render(
                                                         TryAndParse<Galaxy>(
                                                             prev, *this));
                                                 break;
                                               case SimulationMode::
                                                   GalaxiesClashing:
                                                 std::get<GalaxiesClashing>(
                                                     data_variant)
                                                     .Render(TryAndParse<
                                                             GalaxiesClashing>(
                                                         prev, *this));
                                                 break;
                                               case SimulationMode::Uniform:
                                                 std::get<Uniform>(data_variant)
                                                     .Render(
                                                         TryAndParse<Uniform>(
                                                             prev, *this));
                                                 break;
                                               default:
                                                 throw std::exception(
                                                     std::bad_variant_access());
                                                 break;
                                             }
                                           }
                                           ImGui::SameLine();

                                           bool show = prev.has_value() &&
                                                       prev->simulation_type !=
                                                           simulation_type;
                                           if (!show) {
                                             ImGui::BeginDisabled();
                                           }
                                           if (ImGui::Button("Reset")) {
                                             simulation_type =
                                                 prev->simulation_type;
                                             data_variant = prev->data_variant;
                                           }
                                           if (!show) {
                                             ImGui::EndDisabled();
                                           }
                                         }
                                         LayoutResultFunction
                                         LayoutSelector::GetResult() const {
                                           switch (simulation_type) {
                                             case SimulationMode::Galaxy:
                                               return std::get<Galaxy>(
                                                          data_variant)
                                                   .GetResult();
                                               break;
                                             case SimulationMode::
                                                 GalaxiesClashing:
                                               return std::get<
                                                          GalaxiesClashing>(
                                                          data_variant)
                                                   .GetResult();
                                               break;
                                             case SimulationMode::Uniform:
                                               return std::get<Uniform>(
                                                          data_variant)
                                                   .GetResult();
                                               break;
                                             default:
                                               throw std::exception(
                                                   std::bad_variant_access());
                                               break;
                                           }
                                         }

                                         bool Uniform::operator==(const)