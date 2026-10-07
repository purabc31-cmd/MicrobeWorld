#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <vector>

namespace microbeworld {

enum class Species : uint8_t { Algae, Bacteria, Paramecium, Beneficial, Predator, Virus, Toxic };

struct Vec2 { float x=0, y=0; };
struct Organism {
  uint32_t id=0;
  Species species=Species::Bacteria;
  Vec2 pos{};
  Vec2 vel{};
  float radius=6.0f;
  float mass=1.0f;
  float health=100.0f;
  float energy=10.0f;
  float age=0.0f;
};

struct Player {
  Vec2 pos{1300,900};
  Vec2 vel{};
  float radius=24;
  float mass=12;
  float health=100;
  float energy=82;
  float xp=0;
  int evolution=1;
  int eaten=0;
};

class Simulation {
 public:
  Simulation() : rng_(42) {}

  void reset(int mode) {
    mode_ = mode;
    elapsed_ = 0;
    next_id_ = 1;
    organisms_.clear();
    player_ = {};
    player_.pos = {1300, 900};
    player_.radius = 24;
    player_.mass = 12;
    player_.health = 100;
    player_.energy = 82;
    player_.evolution = 1;
    seed();
  }

  void setInput(float x, float y) { input_ = {x, y}; }

  void update(float dt) {
    if (dt <= 0) return;
    elapsed_ += dt;

    const float inputLen = std::hypot(input_.x, input_.y);
    Vec2 dir = inputLen > 0.001f
      ? Vec2{input_.x / inputLen, input_.y / inputLen}
      : Vec2{};

    player_.vel.x += dir.x * 0.11f;
    player_.vel.y += dir.y * 0.11f;
    player_.vel.x *= std::pow(0.90f, dt * 60.0f);
    player_.vel.y *= std::pow(0.90f, dt * 60.0f);

    const float speed = mode_ == 2 ? 4.2f : 3.5f + std::min(1.4f, player_.evolution * .12f);
    player_.pos.x = std::clamp(player_.pos.x + player_.vel.x * speed * dt * 60.0f, player_.radius, 2600.0f - player_.radius);
    player_.pos.y = std::clamp(player_.pos.y + player_.vel.y * speed * dt * 60.0f, player_.radius, 1800.0f - player_.radius);
    player_.energy = std::max(0.0f, player_.energy - dt * (mode_ == 2 ? 0.7f : 1.05f));

    for (auto &o : organisms_) {
      o.age += dt;
      const float dx = player_.pos.x - o.pos.x;
      const float dy = player_.pos.y - o.pos.y;
      const float d = std::max(1.0f, std::hypot(dx, dy));

      if (o.species == Species::Predator) {
        o.vel.x += dx / d * .025f;
        o.vel.y += dy / d * .025f;
      } else if (o.species == Species::Toxic) {
        o.vel.x += std::cos(o.age * 2.1f) * .006f;
        o.vel.y += std::sin(o.age * 1.7f) * .006f;
      } else if (o.species == Species::Paramecium) {
        o.vel.x += std::cos(o.age * 2.3f + o.id) * .012f;
        o.vel.y += std::sin(o.age * 1.9f + o.id) * .012f;
      } else {
        o.vel.x += std::sin(o.age + o.id) * .003f;
        o.vel.y += std::cos(o.age * 1.3f + o.id) * .003f;
      }

      o.vel.x *= .985f;
      o.vel.y *= .985f;
      o.pos.x = std::clamp(o.pos.x + o.vel.x * dt * 45.0f, o.radius, 2600.0f-o.radius);
      o.pos.y = std::clamp(o.pos.y + o.vel.y * dt * 45.0f, o.radius, 1800.0f-o.radius);
    }

    resolveInteractions();
  }

  void evolve() {
    const float cost = static_cast<float>(player_.evolution * 30);
    if (player_.xp < cost || player_.energy < 55) return;
    player_.xp -= cost;
    player_.energy -= 18;
    ++player_.evolution;
    player_.radius *= 1.18f;
    player_.mass *= 1.18f;
    player_.health = std::min(100.0f, player_.health + 18.0f);
  }

  const Player& player() const { return player_; }
  const std::vector<Organism>& organisms() const { return organisms_; }
  float elapsed() const { return elapsed_; }

 private:
  void seed() {
    std::uniform_real_distribution<float> x(40,2560), y(40,1760), v(-.5f,.5f);
    struct SeedCount { Species species; int count; };
    const SeedCount counts[] = {
      {Species::Algae,65},{Species::Bacteria,90},{Species::Paramecium,25},
      {Species::Beneficial,12},{Species::Predator,9},{Species::Virus,16},{Species::Toxic,6}
    };
    for (auto c : counts) {
      for (int i=0; i<c.count; ++i) {
        Organism o;
        o.id = next_id_++;
        o.species = c.species;
        o.pos = {x(rng_), y(rng_)};
        o.vel = {v(rng_), v(rng_)};
        o.radius = baseRadius(c.species);
        o.mass = o.radius * .18f;
        o.energy = baseEnergy(c.species);
        organisms_.push_back(o);
      }
    }
  }

  static float baseRadius(Species s) {
    switch (s) {
      case Species::Algae: return 8;
      case Species::Bacteria: return 6;
      case Species::Paramecium: return 13;
      case Species::Beneficial: return 8;
      case Species::Predator: return 18;
      case Species::Virus: return 10;
      case Species::Toxic: return 15;
    }
    return 6;
  }

  static float baseEnergy(Species s) {
    switch (s) {
      case Species::Algae: return 10;
      case Species::Bacteria: return 8;
      case Species::Paramecium: return 18;
      case Species::Beneficial: return 22;
      default: return 0;
    }
  }

  void resolveInteractions() {
    for (size_t i=0; i<organisms_.size();) {
      auto &o = organisms_[i];
      const float d = std::hypot(player_.pos.x-o.pos.x, player_.pos.y-o.pos.y);
      const bool edible = o.species==Species::Algae || o.species==Species::Bacteria ||
                          o.species==Species::Paramecium || o.species==Species::Beneficial;
      if (edible && d < player_.radius+o.radius && player_.radius >= o.radius*1.12f) {
        player_.mass += std::max(.5f, o.radius*.1f);
        player_.radius = std::sqrt(player_.mass * 2.0f);
        player_.energy = std::min(100.0f, player_.energy + o.energy*.72f);
        player_.xp += o.energy*1.2f;
        ++player_.eaten;
        organisms_.erase(organisms_.begin()+static_cast<long>(i));
        continue;
      }
      if (o.species==Species::Virus && d < player_.radius+o.radius) player_.health -= .07f;
      if (o.species==Species::Toxic && d < player_.radius+o.radius) {
        player_.health -= .10f;
        player_.energy = std::max(0.0f, player_.energy-.03f);
      }
      if (o.species==Species::Predator && d < player_.radius+o.radius && o.radius > player_.radius*1.05f) player_.health -= .38f;
      ++i;
    }
  }

  int mode_=0;
  uint32_t next_id_=1;
  float elapsed_=0;
  Vec2 input_{};
  Player player_{};
  std::vector<Organism> organisms_;
  std::mt19937 rng_;
};

} // namespace microbeworld

// Emscripten-facing C ABI.
// Compile with: em++ microbeworld.cpp -O3 -sMODULARIZE=1 -sEXPORT_ES6=1 //   -sEXPORTED_FUNCTIONS='["_mw_create","_mw_reset","_mw_set_input","_mw_update","_mw_evolve"]' //   -o microbeworld.js
extern "C" {
  microbeworld::Simulation* mw=nullptr;
  void _mw_create(){ if(!mw) mw=new microbeworld::Simulation(); }
  void _mw_reset(int mode){ if(!mw) _mw_create(); mw->reset(mode); }
  void _mw_set_input(float x,float y){ if(!mw) _mw_create(); mw->setInput(x,y); }
  void _mw_update(float dt){ if(!mw) _mw_create(); mw->update(dt); }
  void _mw_evolve(){ if(!mw) _mw_create(); mw->evolve(); }
}