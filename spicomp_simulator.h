#ifndef SPICOMP_SPICOMP_SIMULATOR_H
#define SPICOMP_SPICOMP_SIMULATOR_H

#include <unordered_map>
#include <list>
#include <deque>

#include "util/rng.h"
#include "util/math.h"
#include "util/string_processing.h"

#include "spicomp_setting.h"


#define MAX_MICRO_FRAME_NUM    100
#define BULLET_JUMP_DISTANCE  50.0
#define BULLET_MAX_DISTANCE  600.0
#define INIT_FRAMETREE_LENGTH   20
#define MAX_DRONE_FLIGHT_DISTANCE_PER_FRAME 1000.0

// TODO: MAX_DRONE_FLIGHT_DISTANCE_PER_FRAME is too large


// -------------------------------------------------------------------------------------------

struct Pos3D {
  double x, y, z;

  Pos3D() : x(0.0), y(0.0), z(0.0) {}

  Pos3D(double x, double y, double z) : x(x), y(y), z(z) {}

  Pos3D(const Pos3D& pos) : x(pos.x), y(pos.y), z(pos.z) {}


  void translate(double dx, double dy, double dz) {
    x += dx;
    y += dy;
    z += dz;
  }

  void translate(const Pos3D& displacement) {
    x += displacement.x;
    y += displacement.y;
    z += displacement.z;
  }

  double distance(const Pos3D& pos) const {
    double dx = pos.x - x;
    double dy = pos.y - y;
    double dz = pos.z - z;
    return sqrt(dx * dx + dy * dy + dz * dz);
  }

  friend bool operator==(const Pos3D& pos1, const Pos3D& pos2) {
    return isEqual(pos1.x, pos2.x) && isEqual(pos1.y, pos2.y) && isEqual(pos1.z, pos2.z);
  }

  friend std::ostream& operator<<(std::ostream& out, const Pos3D& pos) {
    out << std::fixed << std::setprecision(1);
    out << '(' << pos.x << " " << pos.y << " " << pos.z << ')';
    return out;
  }

};


struct Color3D {
  int red, green, blue;

  Color3D() : red(0), green(0), blue(0) {}

  Color3D(float red, float green, float blue) : red(red), green(green), blue(blue) {}

  Color3D(const Color3D& color) : red(color.red), green(color.green), blue(color.blue) {}

  void operator=(const Color3D& color) {
    red = color.red;
    green = color.green;
    blue = color.blue;
  }

  friend bool operator==(const Color3D& l, const Color3D& r) {
    return l.red == r.red && l.green == r.green && l.blue == r.blue;
  }

  friend std::ostream& operator<<(std::ostream& out, const Color3D& pos) {
    out << std::fixed << std::setprecision(1);
    out << '(' << pos.red << " " << pos.green << " " << pos.blue << ')';
    return out;
  }

};


inline static const Color3D COLOR_BLACK     {  0,   0,   0};
inline static const Color3D COLOR_GREY      {100, 100, 100};
inline static const Color3D COLOR_RED       {255,   0,   0};
inline static const Color3D COLOR_GREEN     {  0, 255,   0};
inline static const Color3D COLOR_BLUE      {  0,   0, 255};
inline static const Color3D COLOR_DARK_GREEN{  0, 200,   0};
inline static const Color3D COLOR_ORANGE    {255, 165,   0};
inline static const Color3D COLOR_ORANGE_RED{255,  69,   0};

inline static const Color3D COLOR_HIDDEN = COLOR_GREY;

inline static const std::vector<Color3D> COLOR_SET{
  COLOR_BLACK,
  COLOR_GREY,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_BLUE,
  COLOR_DARK_GREEN,
  COLOR_ORANGE,
  COLOR_ORANGE_RED,
};


struct Pixel : public Pos3D, Color3D {

  Pixel(double x, double y, double z, float red, float green, float blue) :
      Pos3D(x, y, z), Color3D(red, green, blue)
  {
    // do nothing
  }

  Pixel(double x, double y, double z, const Color3D& color) :
      Pos3D(x, y, z), Color3D(color)
  {
    // do nothing
  }

  Pixel(const Pos3D& pos, const Color3D& color) :
      Pos3D(pos), Color3D(color)
  {
    // do nothing
  }


  Pos3D& getPos() { return *this; }
  const Pos3D& getPos() const { return *this; }

  Color3D& getColor() { return *this; }
  const Color3D& getColor() const { return *this; }

  friend bool operator==(const Pixel& pixel1, const Pixel& pixel2) {
    return static_cast<Pos3D>(pixel1) == static_cast<Pos3D>(pixel2) && static_cast<Color3D>(pixel1) == static_cast<Color3D>(pixel2);
  }

  friend std::ostream& operator<<(std::ostream& out, Pixel& pixel) {
    out << '[' << static_cast<Pos3D>(pixel) << " " << static_cast<Color3D>(pixel) << ']';
    return out;
  }
};


// -------------------------------------------------------------------------------------------
//   Formation
// -------------------------------------------------------------------------------------------

class DroneState {

  Pos3D pos;
  Color3D color;
  bool isHidden;

public:

  DroneState(double x, double y, double z) :
      pos(x, y, z), color(COLOR_HIDDEN), isHidden(false)
  {
    // do nothing
  }

  DroneState(double x, double y, double z, const Color3D& color) :
      pos(x, y, z), color(color), isHidden(color==COLOR_HIDDEN)
  {
    // do nothing
  }

  DroneState(const Pixel& pixel) :
      pos(pixel.x, pixel.y, pixel.z), color(pixel), isHidden(color==COLOR_HIDDEN)
  {
    // do nothing
  }

  explicit DroneState(const DroneState& state) : pos(state.pos), color(state.color), isHidden(state.isHidden) {}

  void operator=(const DroneState& state) {
    pos = state.pos;
    color = state.color;
    isHidden = state.isHidden;
  }


  const Pos3D& getPos() const { return pos; }
  const Color3D getColor() const { return color; }
  bool getIsHidden() const { return isHidden; }

  Pixel getPixel() const {
    return Pixel(pos.x, pos.y, pos.z, color);
  }


  void setPos(const Pos3D& pos) { DroneState::pos = pos; }
  void setPos(double x, double y, double z) { DroneState::pos = Pos3D(x, y, z); }

  void setColor(const Color3D& color) {
    DroneState::color = color;
    isHidden = (color == COLOR_HIDDEN);
  }

};


// -------------------------------------------------------------------------------------------
//   Frame
// -------------------------------------------------------------------------------------------

class Frame {

  int id;
  std::vector<Pixel> pixel_db;

public:

  Frame() = default;
  Frame(const Frame& frame) : id(frame.id), pixel_db(frame.pixel_db) {}

  void operator=(const Frame& frame) { id = frame.id; pixel_db = frame.pixel_db; }


  int getId() const { return id; }
  int size() const { return pixel_db.size(); }

  const Pixel& getPixel(int pixel_id) const { return pixel_db[pixel_id]; }
  const std::vector<Pixel>& getPixels() const { return pixel_db; }


  void setId(int id) { Frame::id = id; }

  void addPixel(double x, double y, double z, const Color3D& color) {
    pixel_db.emplace_back(x, y, z, color);
  }

  void addPixel(const Pixel& pixel) { pixel_db.push_back(pixel); }

  void translate(double x, double y, double z) {
    for(auto& pixel : pixel_db) {
      pixel.translate(x, y, z);
    }
  }

  friend std::ostream& operator<<(std::ostream& out, const Frame& frame) {
    out << "{Frame" << frame.id << ": ";
    out << to_string(frame.pixel_db);
    out << '}';
    return out;
  }

};


// -------------------------------------------------------------------------------------------
//   Drone Assignment
// -------------------------------------------------------------------------------------------

using DroneAssignment = std::vector<int>;


// -------------------------------------------------------------------------------------------
//   The Frame Sequence
// -------------------------------------------------------------------------------------------

using FrameSeq = std::vector<Frame>;

void translate(FrameSeq& frame, double x, double y, double z);


// -------------------------------------------------------------------------------------------
//   The Decision Variable
// -------------------------------------------------------------------------------------------

using DecisionOption = int;

class DecisionVariable {

  int id;
  std::vector<DecisionOption> domains;
  DecisionOption default_option;

public:

  DecisionVariable() : id{-1}, default_option{DecisionVariable::NIL} {}

  DecisionVariable(int id, const std::vector<DecisionOption>& domains, DecisionOption default_option) : id(id), domains(domains), default_option(default_option) {
    assert(!domains.empty());
    assert(std::find(domains.begin(), domains.end(), default_option) != domains.end());
  }


  bool isExist() const { return id != -1; }
  bool isNil() const { return default_option == DecisionVariable::NIL; }

  int getId() const { return id; }
  int size() const { return domains.size(); }
  const std::vector<DecisionOption>& getDomains() const { return domains;}
  const DecisionOption& getDefaultOption() const { return default_option; }
  bool contains(const DecisionOption& option) const { return std::find(domains.begin(), domains.end(), option) != domains.end(); }

  bool isSubdomainOf(const DecisionVariable& decision_variable) const {
    for(auto& option : domains) {
      if (!decision_variable.contains(option)) return false;
    }
    return true;
  }

  friend std::ostream& operator<<(std::ostream& out, const DecisionVariable& decision_variable) {
    if (decision_variable.isExist()) {
      out << "<v" << decision_variable.id << "=" << decision_variable.default_option << ">={";
      out << to_string(decision_variable.domains);
      out << '}';
    } else {
      out << "<v?=nil>";
    }
    return out;
  }

  inline static DecisionOption NIL = -1;
};


// -------------------------------------------------------------------------------------------
//   The Frame Tree
// -------------------------------------------------------------------------------------------

class FrameTree {

  int root_frame_id;
  std::unordered_map<int, Frame> frame_db;
  std::unordered_map<int, DecisionVariable> frame_decision_var_db;
  std::unordered_map<int, std::unordered_map<DecisionOption,int>> children_ids_db;
  std::unordered_map<int, int> parent_frame_id_db;
  std::unordered_map<int, DecisionOption> parent_option_db;

public:

  FrameTree() : root_frame_id(-1) {}

  bool empty() const { return frame_db.empty(); }   // root_frame_id = -1 as well
  int size() const { return frame_db.size(); }
  void clear() {
    root_frame_id = -1;
    frame_db.clear();
    frame_decision_var_db.clear();
    children_ids_db.clear();
    parent_frame_id_db.clear();
    parent_option_db.clear();
  }

  // --- Query ---

  int getRootFrameId() const { return root_frame_id; }
  const Frame& getRootFrame() const { return frame_db.at(root_frame_id); }

  bool isFrameExist(int frame_id) const { return frame_db.contains(frame_id); }
  const Frame& getFrame(int frame_id) const { return frame_db.at(frame_id); }
  const std::unordered_map<int, Frame>& getAllFrames() const { return frame_db; }

  bool isDecisionFrame(int frame_id) const { return frame_decision_var_db.contains(frame_id); }
  const DecisionVariable& getDecisionVariable(int frame_id) const { return frame_decision_var_db.at(frame_id); }
  bool hasOption(int frame_id, const DecisionOption& option) const { return getDecisionVariable(frame_id).contains(option); }
  const DecisionOption& getDefaultOption(int frame_id) const { return getDecisionVariable(frame_id).getDefaultOption(); }

  bool isTerminalFrame(int frame_id) const { return !children_ids_db.contains(frame_id); }
  bool hasChildOption(int frame_id, const DecisionOption& option) const { return children_ids_db.at(frame_id).contains(option); }
  bool hasChildNilOption(int frame_id) const { return hasChildOption(frame_id, DecisionVariable::NIL); }
  bool hasChildFrameId(int frame_id, int child_id) const {
    for(auto [ option, child_id_2]: children_ids_db.at(frame_id)) { if (child_id_2 == child_id) return true; }
    return false;
  }
  bool hasChildFrameIdWithOption(int frame_id, const DecisionOption& option, int child_id) const {
    return !isTerminalFrame(frame_id) && hasChildOption(frame_id, option) && children_ids_db.at(frame_id).at(option) == child_id;
  }
  bool hasChildFrameIdWithoutOption(int frame_id, int child_id) const {
    return !isTerminalFrame(frame_id) && hasChildNilOption(frame_id) && children_ids_db.at(frame_id).at(DecisionVariable::NIL) == child_id;
  }

  int getChildFrameId(int frame_id, const DecisionOption& option) const { assert(isDecisionFrame(frame_id)); return children_ids_db.at(frame_id).at(option); }
  int getUniqueChildFrameId(int frame_id) const { assert(!isDecisionFrame(frame_id)); return children_ids_db.at(frame_id).at(DecisionVariable::NIL); }
  int getDefaultChildFrameId(int frame_id) const { assert(isDecisionFrame(frame_id)); return children_ids_db.at(frame_id).at(getDecisionVariable(frame_id).getDefaultOption()); }
  const Frame& getChildFrame(int frame_id, const DecisionOption& option) const { assert(isDecisionFrame(frame_id)); return getFrame(getChildFrameId(frame_id, option)); }
  const Frame& getUniqueChildFrame(int frame_id) const { assert(!isDecisionFrame(frame_id)); return getFrame(getUniqueChildFrameId(frame_id)); }
  const Frame& getDefaultChildFrame(int frame_id) const { assert(isDecisionFrame(frame_id)); return getFrame(getDefaultChildFrameId(frame_id)); }
  const std::unordered_map<DecisionOption,int>& getAllChildrenIdsWithOptions(int frame_id) const { return children_ids_db.at(frame_id); }

  int hasParentFrameId(int frame_id) const { return parent_frame_id_db.contains(frame_id); }
  int getParentFrameId(int frame_id) const { return parent_frame_id_db.at(frame_id); }
  int getParentOption(int frame_id) const { return parent_option_db.at(frame_id); }

  // --- Update ---

  void setRootFrameId(int frame_id) {
    assert(root_frame_id == -1);
    root_frame_id = frame_id;
  }

  void addFrame(const Frame& frame) {
    auto frame_id = frame.getId();
    assert(!isFrameExist(frame_id));
    assert(!isDecisionFrame(frame_id));  // must not have the decision variable before adding the frame
    assert(isTerminalFrame(frame_id));   // must not have any children before adding the frame
    frame_db[frame_id] = frame;
  }

  void removeFrame(int frame_id) {
    assert(isFrameExist(frame_id));
    assert(!isDecisionFrame(frame_id));  // must remove the decision variable before removing the frame
    assert(isTerminalFrame(frame_id));   // must remove all children before removing the frame
    frame_db.erase(frame_id);
    if (frame_id == root_frame_id) { root_frame_id = -1; } // need to use setRootFrameId() to update root_frame_id after calling removeFrame()
  }

  void setDecisionVariable(int frame_id, const DecisionVariable& decision_var) {
    assert(isFrameExist(frame_id));
    assert(!isDecisionFrame(frame_id));
    assert(isTerminalFrame(frame_id));   // must not have any children before adding the decision variable
    frame_decision_var_db.insert({frame_id, decision_var} );
  }

  void removeDecisionVariable(int frame_id) {
    assert(isFrameExist(frame_id));
    assert(isDecisionFrame(frame_id));
    assert(isTerminalFrame(frame_id));   // must remove all children before removing the decision variable
    frame_decision_var_db.erase(frame_id);
  }

  void addChildId(int frame_id, const DecisionOption& option, int child_id) {
    assert(isFrameExist(frame_id));
    assert(isFrameExist(child_id));
    assert(isDecisionFrame(frame_id));
    assert(hasOption(frame_id, option));
    assert(!hasChildFrameIdWithOption(frame_id, option, child_id));
    assert(!hasParentFrameId(child_id));
    children_ids_db[frame_id][option] = child_id;
    parent_frame_id_db[child_id] = frame_id;
    parent_option_db[child_id] = option;
  }

  void addUniqueChildId(int frame_id, int child_id) {
    assert(isFrameExist(frame_id));
    assert(isFrameExist(child_id));
    assert(!isDecisionFrame(frame_id));
    assert(!hasChildFrameIdWithoutOption(frame_id, child_id));
    assert(!hasParentFrameId(child_id));
    children_ids_db[frame_id][DecisionVariable::NIL] = child_id;
    parent_frame_id_db[child_id] = frame_id;
    parent_option_db[child_id] = DecisionVariable::NIL;
  }

  void removeChildId(int frame_id, const DecisionOption& option, int child_id) {
    assert(isFrameExist(frame_id));
    assert(isFrameExist(child_id));
    assert(isDecisionFrame(frame_id));
    assert(hasOption(frame_id, option));
    assert(hasChildFrameIdWithOption(frame_id, option, child_id));
    assert(hasParentFrameId(child_id));
    children_ids_db[frame_id].erase(option);
    parent_frame_id_db.erase(child_id);
    parent_option_db.erase(child_id);
    if (children_ids_db[frame_id].empty()) {
      children_ids_db.erase(frame_id);
    }
  }

  void removeUniqueChildId(int frame_id, int child_id) {
    assert(isFrameExist(frame_id));
    assert(isFrameExist(child_id));
    assert(!isDecisionFrame(frame_id));
    assert(hasChildFrameIdWithoutOption(frame_id, child_id));
    assert(hasParentFrameId(child_id));
    children_ids_db[frame_id].erase(DecisionVariable::NIL);
    parent_frame_id_db.erase(child_id);
    parent_option_db.erase(child_id);
    if (children_ids_db[frame_id].empty()) {
      children_ids_db.erase(frame_id);
    }
  }

  void attachFrameSubtreeToTerminalFrame(const FrameTree& subtree) {
    attachFrameSubtreeToTerminalFrame(subtree, subtree.getRootFrameId());
    assert(isValid());
  }

  void attachFrameSubtree(const FrameTree& subtree, int subtree_root_frame_id, const DecisionVariable& new_decision_variable,
                          DecisionOption option_for_unique_child_in_original, DecisionOption option_for_unique_child_in_subtree);  // only when subtree_root_frame is not a terminal frame in this tree.    // TODO: not tested yet


  void deleteFrameSubtree(int frame_id);

  void pop_front();


  bool isValid() const;

  void print() const;

private:

  void attachFrameSubtreeToTerminalFrame(const FrameTree& subtree, int subtree_root_frame_id);   // only when subtree_root_frame is a terminal frame in this tree.    // TODO: not tested yet

  bool isValid(int frame_id, std::vector<int>& visited_frame_ids) const;

  void print(int indent_size, int frame_id) const;

};


// -------------------------------------------------------------------------------------------
//   Formation
// -------------------------------------------------------------------------------------------

class Formation {

  std::vector<DroneState> drone_state_db;

public:

  Formation() = default;

  Formation(const Formation& formation) : drone_state_db(formation.drone_state_db) {}

  void operator=(const Formation& formation2) { drone_state_db = formation2.drone_state_db; }


  int size() const { return drone_state_db.size(); }

  DroneState& getDroneState(int drone_id) { return drone_state_db[drone_id]; }
  const DroneState& getDroneState(int drone_id) const { return drone_state_db[drone_id]; }
  const std::vector<DroneState>& getDroneStates() const { return drone_state_db; }

  const Frame makeFrame() const {
    Frame frame;
    for(auto& drone_state : drone_state_db) {
      frame.addPixel(drone_state.getPixel());
    }
    return frame;
  }

  void clear() { drone_state_db.clear(); }

  void addDroneState(double x, double y, double z) {
    drone_state_db.emplace_back(x, y, z);
  }

  void addDroneState(double x, double y, double z, const Color3D& color) {
    drone_state_db.emplace_back(x, y, z, color);
  }

  void addDroneState(const Pixel& pixel) {
    drone_state_db.emplace_back(pixel);
  }

};


// -------------------------------------------------------------------------------------------
//   Formation Plan
// -------------------------------------------------------------------------------------------

class FormationPlan {

  Formation formation1;
  std::vector<Formation> micro_formation_seq;  // excluding formation1

  int frame1_id;
  int frame2_id;

  DroneAssignment assignment1; // duplicated from the parent formation plan
  DroneAssignment assignment2;

public:

  FormationPlan() : frame1_id(-1), frame2_id(-1) {}

  FormationPlan(int frame1_id, int frame2_id) : frame1_id{frame1_id}, frame2_id{frame2_id} {}

  bool isNil() const { frame1_id == -1 && frame2_id == -1; }

  bool size() const { return micro_formation_seq.size(); }
  bool empty() const { return micro_formation_seq.empty(); }

  const Formation& getFormation1() const { return formation1; }
  const Formation& getFormation2() const { return micro_formation_seq.back(); }

  Formation& getMicroFormation(int formation_id) { return micro_formation_seq[formation_id]; }
  const Formation& getMicroFormation(int formation_id) const { return micro_formation_seq[formation_id]; }

  int getFrame1Id() const { return frame1_id; }
  int getFrame2Id() const { return frame2_id; }
  const DroneAssignment& getAssignment1() const { return assignment1; }
  const DroneAssignment& getAssignment2() const { return assignment2; }
  DroneAssignment& getAssignment2() { return assignment2; }

  void addMicroFormation(const Formation& formation) { micro_formation_seq.push_back(formation); }
  void setFormation1(const Formation& formation1) { FormationPlan::formation1 = formation1; }
  void setAssignment1(const DroneAssignment& assignment1) { FormationPlan::assignment1 = assignment1; }
  void setAssignment2(const DroneAssignment& assignment2) { FormationPlan::assignment2 = assignment2; }

  friend std::ostream& operator<<(std::ostream& out, const FormationPlan& formation_plan) {
    out << "FormationPlan {micro_formation_seq.size()=" << formation_plan.micro_formation_seq.size() << "}";
    return out;
  }

};


// -------------------------------------------------------------------------------------------
//   Game State
// -------------------------------------------------------------------------------------------


class GameState {

  static const std::vector<Pos3D> gun_trajectory;

  int id;
  int pos_id;
  int power_level_id;
  std::vector<Pos3D> bullet_pos_list;

public:

  GameState() : id{-1}, pos_id{-1}, power_level_id{-1} { }

  GameState(int& id) : GameState(id, 0, 0, {}) { } // no need to increase id by 1

  GameState(int& id, int pos_id, int power_level_id, const std::vector<Pos3D>& bullet_pos_list) :
      id{id}, pos_id{pos_id}, power_level_id{power_level_id}, bullet_pos_list{bullet_pos_list}
  {
    id++;
  }

  int getId() const { return id; }

  bool isDecisionGameState() const {
    return power_level_id == 2;
  }

  DecisionVariable getDecisionVariable(int& next_decision_variable_id) const {
    assert(isDecisionGameState());
    // auto default_option = SharedRand::getRandInt(2);
    auto default_option = 1;
    DecisionVariable v(next_decision_variable_id, {0, 1}, default_option);   // ******
    next_decision_variable_id++;  // advance to next decision variable id
    return v;
  }

  [[nodiscard]] std::unordered_map<DecisionOption,GameState> getNextGameStates(const DecisionVariable& decision_variable, int& next_id) const {
    assert(isDecisionGameState());

    int next_pos_id = pos_id + 1;
    if (next_pos_id >= gun_trajectory.size()) { next_pos_id = 0; }

    int next_power_level_id = power_level_id + 1;
    if (next_power_level_id >= 4) { next_power_level_id = 0; }

    auto next_bullet_pos_list0 = advanceBullets(bullet_pos_list);
    auto new_bullet_pos = gun_trajectory[next_pos_id];
    new_bullet_pos.translate(50.0, 50.0, 125.0);
    auto next_bullet_pos_list1 = addNewBullet(next_bullet_pos_list0, new_bullet_pos);

    GameState state0(next_id, next_pos_id, next_power_level_id, next_bullet_pos_list0);
    GameState state1(next_id, next_pos_id, next_power_level_id, next_bullet_pos_list1);

    return { {0, state0}, {1, state1}};
  }

  GameState getUniqueNextGameState(int& next_id) const {    // when isDecisionGameState is false
    assert(!isDecisionGameState());

    int next_pos_id = pos_id + 1;
    if (next_pos_id >= gun_trajectory.size()) { next_pos_id = 0; }

    int next_power_level_id = power_level_id + 1;
    if (next_power_level_id >= 4) { next_power_level_id = 0; }

    GameState state(next_id, next_pos_id, next_power_level_id, advanceBullets(bullet_pos_list));

    return state;
  }


  Frame makeFrame() const {

    std::vector<Pixel> gun_pixels;

    auto gun_color = COLOR_GREEN;
    auto power_color = COLOR_RED;

    std::vector<Color3D> gun_colors(3, gun_color);
    if (power_level_id < 3) {
      gun_colors[power_level_id] = power_color;
    }

    gun_pixels.emplace_back(  0.0,   0.0,  0.0, gun_colors[0]);
    gun_pixels.emplace_back( 50.0,   0.0,  0.0, gun_colors[0]);
    gun_pixels.emplace_back(100.0,   0.0,  0.0, gun_colors[0]);
    gun_pixels.emplace_back(  0.0,  50.0,  0.0, gun_colors[0]);
    gun_pixels.emplace_back(  0.0, 100.0,  0.0, gun_colors[0]);
    gun_pixels.emplace_back( 50.0, 100.0,  0.0, gun_colors[0]);
    gun_pixels.emplace_back(100.0,  50.0,  0.0, gun_colors[0]);
    gun_pixels.emplace_back(100.0, 100.0,  0.0, gun_colors[0]);

    gun_pixels.emplace_back( 25.0,  25.0, 50.0, gun_colors[1]);
    gun_pixels.emplace_back( 75.0,  25.0, 50.0, gun_colors[1]);
    gun_pixels.emplace_back( 25.0,  75.0, 50.0, gun_colors[1]);
    gun_pixels.emplace_back( 75.0,  75.0, 50.0, gun_colors[1]);

    gun_pixels.emplace_back( 50.0,  50.0, 100.0, gun_colors[2]);

    for(auto& pixel : gun_pixels) {
      pixel.translate(gun_trajectory[pos_id]);
    }

    Frame frame;
    frame.setId(id);  // make frame_id equal to game_state_id

    for(auto& pixel : gun_pixels) {
      frame.addPixel(pixel);
    }

    for(auto& bullet_pos : bullet_pos_list) {
//      Pixel bullet_pixel(bullet_pos, COLOR_ORANGE_RED);
//      frame.addPixel(bullet_pixel);

      auto bullet_pos_down = bullet_pos;
      bullet_pos_down.translate(0.0, 0.0, -BULLET_JUMP_DISTANCE / 4.0);
      Pixel bullet_pixel_down(bullet_pos_down, COLOR_ORANGE_RED);
      frame.addPixel(bullet_pixel_down);

      auto bullet_pos_up = bullet_pos;
      bullet_pos_up.translate(0.0, 0.0, BULLET_JUMP_DISTANCE / 4.0);
      Pixel bullet_pixel_up(bullet_pos_up, COLOR_ORANGE_RED);
      frame.addPixel(bullet_pixel_up);
    }

    return frame;
  }

private:

  static std::vector<Pos3D> advanceBullets(const std::vector<Pos3D>& bullet_pos_list) {
    std::vector<Pos3D> next_bullet_pos_list;
    for(auto& bullet_pos : bullet_pos_list) {
      if (bullet_pos.z + BULLET_JUMP_DISTANCE <= BULLET_MAX_DISTANCE) {
        next_bullet_pos_list.push_back(bullet_pos);
        next_bullet_pos_list.back().translate(0.0, 0.0, BULLET_JUMP_DISTANCE);
      }  // else drop this bullet
    }
    return next_bullet_pos_list;
  }

  static std::vector<Pos3D> addNewBullet(const std::vector<Pos3D>& bullet_pos_list, Pos3D bullet_pos) {
    std::vector<Pos3D> next_bullet_pos_list = bullet_pos_list;
    next_bullet_pos_list.push_back(bullet_pos);
    return next_bullet_pos_list;
  }


};


// -------------------------------------------------------------------------------------------
//   Game State
// -------------------------------------------------------------------------------------------


class GameStateTree {

  int root_game_state_id;
  std::unordered_map<int, GameState> game_state_db;
  std::unordered_map<int, DecisionVariable> decision_variable;
  std::unordered_map<int, std::unordered_map<DecisionOption,int>> children_ids;

public:

  GameStateTree() : root_game_state_id(-1) {}

  bool empty() const { return root_game_state_id == -1; }
  void clear() { root_game_state_id = -1; game_state_db.clear(); decision_variable.clear(); children_ids.clear(); }
  int size() const { return game_state_db.size(); }

  int getRootGameStateId() const { return root_game_state_id; }
  const GameState& getRootGameState() const { return game_state_db.at(root_game_state_id); }

  bool isGameStateExist(int game_state_id) const { return game_state_db.contains(game_state_id); }
  const GameState& getGameState(int game_state_id) const { return game_state_db.at(game_state_id); }
  const std::unordered_map<int, GameState>& getAllGameStates() const { return game_state_db; }

  bool isDecisionGameState(int game_state_id) const { return decision_variable.contains(game_state_id); }
  const DecisionVariable& getDecisionVariable(int game_state_id) const { return decision_variable.at(game_state_id); }

  bool isTerminalGameState(int game_state_id) const { return !children_ids.contains(game_state_id); }

  std::vector<int> getAllTerminalGameStateIds() const {
    std::vector<int> terminal_game_state_ids;
    getAllTerminalGameStateIds(terminal_game_state_ids, root_game_state_id);
    return terminal_game_state_ids;
  }

  int getChildGameStateId(int game_state_id) const { return children_ids.at(game_state_id).at(DecisionVariable::NIL); }
  int getChildGameStateId(int game_state_id, const DecisionOption& option) const { return children_ids.at(game_state_id).at(option); }
  int getDefaultChildGameStateId(int game_state_id) const { return children_ids.at(game_state_id).at(getDecisionVariable(game_state_id).getDefaultOption()); }

  const GameState& getChildGameState(int game_state_id) const { return getGameState(getChildGameStateId(game_state_id)); }
  const GameState& getChildGameState(int game_state_id, const DecisionOption& option) const { return getGameState(getChildGameStateId(game_state_id, option)); }
  const GameState& getDefaultChildGameState(int game_state_id) const { return getGameState(getDefaultChildGameStateId(game_state_id)); }

  const std::unordered_map<DecisionOption,int>& getChildrenIds(int game_state_id) const { return children_ids.at(game_state_id); }


  void setRootGameStateId(int game_state_id) { root_game_state_id = game_state_id; }

  void addGameState(const GameState& game_state) {
    game_state_db[game_state.getId()] = game_state;
  }

  void setDecisionVariable(int game_state_id, const DecisionVariable& decision_var) {
    assert(!decision_variable.contains(game_state_id));
    decision_variable.insert( {game_state_id, decision_var} );
  }

  void removeDecisionVariable(int game_state_id) {
    assert(decision_variable.contains(game_state_id));
    decision_variable.erase(game_state_id);
  }

  void addChildrenId(int game_state_id, int child_id) {
    assert(!decision_variable.contains(game_state_id));
    children_ids[game_state_id][DecisionVariable::NIL] = child_id;
  }

  void addChildrenId(int game_state_id, const DecisionOption& option, int child_id) {
    assert(decision_variable.contains(game_state_id));
    assert(decision_variable.at(game_state_id).contains(option));
    children_ids[game_state_id][option] = child_id;
  }

  void pop_front();


private:

  void deleteGameStateSubtree(int game_state_id);

  void getAllTerminalGameStateIds(std::vector<int>& terminal_game_state_ids, int game_state_id) const {
    if (isTerminalGameState(game_state_id)) {
      terminal_game_state_ids.push_back(game_state_id);
    } else {
      for(auto [option, child_id] : getChildrenIds(game_state_id)) {
        getAllTerminalGameStateIds(terminal_game_state_ids, child_id);
      }
    }
  }

};


// -------------------------------------------------------------------------------------------
//   The Game Controller
// -------------------------------------------------------------------------------------------

class GameController {

  const int micro_frame_num;

  int sim_step_count;
  int next_game_state_id;
  int next_decision_variable_id;
  int pixel_trajectory_tracking_num;

  std::unordered_map<int,DecisionVariable> decision_variable_list;  // TODO: remove some of this over time

  GameStateTree game_state_tree;

public:

  GameController(int micro_frame_num) : micro_frame_num{micro_frame_num} {
    reset();
  }

  void reset() {
    sim_step_count = 0;
    next_game_state_id = 0;
    next_decision_variable_id = 0;
    game_state_tree = GameStateTree();
    GameState root_game_state(next_game_state_id);
    game_state_tree.addGameState(root_game_state);
    game_state_tree.setRootGameStateId(root_game_state.getId());
    pixel_trajectory_tracking_num = game_state_tree.getRootGameState().makeFrame().size();
  }

  void nextStep() {
    sim_step_count++;
  }

  int size() const { return game_state_tree.size(); }

  int getPixelTrajectoryTrackingNum() const { return pixel_trajectory_tracking_num; }

  FrameTree getInitFrameTree() {
    FrameTree original_frame_tree;
    makeFrameTree(original_frame_tree, game_state_tree.getRootGameStateId());
    for(int i=0; i<INIT_FRAMETREE_LENGTH; i++) {
      extendFrameTree(original_frame_tree);
    }
    return original_frame_tree;
  }

  std::vector<FrameTree> getNewFrameTrees() {
    if (sim_step_count % micro_frame_num == (micro_frame_num-1)) {
      std::vector<FrameTree> frame_tree_list;
      for(auto game_state_id : game_state_tree.getAllTerminalGameStateIds()) {
        frame_tree_list.emplace_back();
        auto& frame_tree = frame_tree_list.back();
        makeFrameTree(frame_tree, game_state_id);
      }
      return frame_tree_list;
    } else {
      return {};
    }
  }

  void removeFirstGameState() {
    assert(sim_step_count % micro_frame_num == (micro_frame_num-1));
    game_state_tree.pop_front();
  }

private:

  void makeFrameTree(FrameTree& frame_tree, int game_state_id) {
    assert(game_state_tree.isTerminalGameState(game_state_id));

    auto& game_state = game_state_tree.getGameState(game_state_id);
    auto frame1 = game_state.makeFrame();
    frame_tree.addFrame(frame1);

    frame_tree.setRootFrameId(frame1.getId());

    if (game_state.isDecisionGameState()) {
      auto decision_variable = game_state.getDecisionVariable(next_decision_variable_id);
      assert(decision_variable.getId() == next_decision_variable_id-1);
      decision_variable_list.insert({ decision_variable.getId(), decision_variable });
      game_state_tree.setDecisionVariable(game_state.getId(), decision_variable);
      frame_tree.setDecisionVariable(frame1.getId(), decision_variable);
      for(auto& [option, next_game_state] : game_state.getNextGameStates(decision_variable, next_game_state_id)) {
        auto frame2 = next_game_state.makeFrame();
        game_state_tree.addGameState(next_game_state);
        frame_tree.addFrame(frame2);
        game_state_tree.addChildrenId(game_state.getId(), option, next_game_state.getId());
        frame_tree.addChildId(frame1.getId(), option, frame2.getId());
      }
    } else {
      auto next_game_state = game_state.getUniqueNextGameState(next_game_state_id);
      auto frame2 = next_game_state.makeFrame();
      game_state_tree.addGameState(next_game_state);
      frame_tree.addFrame(frame2);
      game_state_tree.addChildrenId(game_state.getId(), next_game_state.getId());
      frame_tree.addUniqueChildId(frame1.getId(), frame2.getId());
    }
  }

  void extendFrameTree(FrameTree& original_frame_tree) {
    for(auto game_state_id : game_state_tree.getAllTerminalGameStateIds()) {
      FrameTree frame_tree;
      makeFrameTree(frame_tree, game_state_id);
      original_frame_tree.attachFrameSubtreeToTerminalFrame(frame_tree);
    }
  }

};


// -------------------------------------------------------------------------------------------
//   The Frame Buffer
// -------------------------------------------------------------------------------------------

class FrameBuffer {

  const int micro_frame_num;

  int sim_step_count;

  FrameTree frame_tree;

public:

  FrameBuffer(int micro_frame_num) : micro_frame_num{micro_frame_num}, sim_step_count{0} { reset(); }

  void reset() {
    sim_step_count = 0;
    frame_tree.clear();
  }

  void nextStep() {
    sim_step_count++;
  }

  int size() { return frame_tree.size(); }

  const FrameTree& getFrameTree() const { return frame_tree; }


  void setFrameTree(const FrameTree& new_frame_tree) {
    frame_tree = new_frame_tree;
  }

  void attachFrameTree(const FrameTree& new_frame_tree) {
    frame_tree.attachFrameSubtreeToTerminalFrame(new_frame_tree);
  }

  void removeFirstFrame() {
    frame_tree.pop_front();
  }


};


// -------------------------------------------------------------------------------------------
//   Contingency Formation Plan
// -------------------------------------------------------------------------------------------

class ContingencyFormationPlan {

  std::unordered_map<int,std::unordered_map<int,FormationPlan>> formation_plan_db;

public:

  void clear() { formation_plan_db.clear(); }

  bool isFormationPlanExist(int frame1_id, int frame2_id) const {
    if (!formation_plan_db.contains(frame1_id)) return false;
    return formation_plan_db.at(frame1_id).contains(frame2_id);
  }

  const FormationPlan& getFormationPlan(int frame1_id, int frame2_id) const { return formation_plan_db.at(frame1_id).at(frame2_id); }

  FormationPlan& getFormationPlan(int frame1_id, int frame2_id) { return formation_plan_db.at(frame1_id).at(frame2_id); }

  void addFormationPlan(int frame1_id, int frame2_id, const FormationPlan& plan) {
    assert(!isFormationPlanExist(frame1_id, frame2_id));
    formation_plan_db[frame1_id][frame2_id] = plan;
  }

  void emplaceFormationPlan(int frame1_id, int frame2_id) {
    assert(!isFormationPlanExist(frame1_id, frame2_id));
    formation_plan_db[frame1_id].insert({frame2_id, FormationPlan(frame1_id, frame2_id)});
  }


  void print() const {
    __pp__("ContingencyFormationPlan::print():");
    for(auto& [frame1_id, formation_plan_db2] : formation_plan_db) {
      for(auto& [frame2_id, formation_plan] : formation_plan_db2) {
        std::cout << "frame" << frame1_id << " -> frame2" << frame2_id << " : " << formation_plan << std::endl;
      }
    }
    std::cout << std::endl;
  }
};


// -------------------------------------------------------------------------------------------
//   The SPICOMP algorithm
// -------------------------------------------------------------------------------------------

class SpicompPlanner {

  const int drone_num;
  const int micro_frame_num;

  const FrameTree& frame_tree;
  const Formation& init_formation;
  const DroneAssignment& init_assignment;
  const ContingencyFormationPlan& previous_cf_plan;

  const int pixel_trajectory_tracking_num;   // TODO: for now, we assume the number of pixel trajectory tracking pixels is fixed.

  ContingencyFormationPlan cf_plan;  // TODO: need to clean up formation plan in cf_plan

public:

  SpicompPlanner(int drone_num, int micro_frame_num, const FrameTree& frame_tree, const Formation& init_formation, const DroneAssignment& init_assignment,
                 const ContingencyFormationPlan& previous_cf_plan, int pixel_trajectory_tracking_num) :
      drone_num{drone_num}, micro_frame_num{micro_frame_num},
      frame_tree{frame_tree}, init_formation{init_formation}, init_assignment{init_assignment},
      previous_cf_plan(previous_cf_plan),
      pixel_trajectory_tracking_num{pixel_trajectory_tracking_num}
  {
    assert(init_formation.size() == drone_num);
    solve();
  }

  const ContingencyFormationPlan& getContingencyFormationPlan() const { return cf_plan; }


private:

  bool solve() {
    cf_plan.clear();
    return solve(frame_tree.getRootFrameId(), init_formation, init_assignment, 0);
  }

  bool solve(int frame_id, const Formation& formation, const DroneAssignment& assignment, int search_depth);

  bool computeFormationPlan(FormationPlan& fplan, const Frame& frame1, const Frame& frame2, const Formation& formation1, const DroneAssignment& assignment1);

  void computeEarliestAvailableMicroFormations(FormationPlan& fplan, int drone_id, const Pixel& pixel2, int pixel2_id, const std::vector<int>& parent_id_list);

  void computeLinearMicroFormations(FormationPlan& fplan, int drone_id, const Pixel& pixel1, const Pixel& pixel2);

  void computeGoDarkMicroFormations(FormationPlan& fplan, int drone_id, const Pixel& pixel1);


  std::list<int> findUnassignedDroneIds(const std::vector<int>& assignment2) const;

  static std::list<int>::const_iterator findRandomNearbyDroneId(const Pos3D& pixel_pos, const Formation& formation1, const std::list<int>& unassigned_drone_ids);

  static std::list<int>::const_iterator findRandomEarliestAvailableDroneId(const Pos3D& pixel_pos, const Formation& formation1, const std::list<int>& unassigned_drone_ids, const std::unordered_map<int, std::vector<int>>& earliest_available_frame_ids_db);


  std::vector<int> findEarliestAvailableFrameId(const FormationPlan& fplan, int drone_id) const {
    std::vector<int> parent_frame_id_list;
    findEarliestAvailableFrameId(parent_frame_id_list, fplan, drone_id);
    std::reverse(parent_frame_id_list.begin(), parent_frame_id_list.end());
    return parent_frame_id_list;
  }

  void findEarliestAvailableFrameId(std::vector<int>& parent_frame_id_list, const FormationPlan& fplan, int drone_id) const;


  static bool isDroneAssigned(int drone_id, const DroneAssignment& assignment) {
    return std::find(assignment.begin(), assignment.end(), drone_id) != assignment.end();
  }

  static int findAssignedPixelId(int drone_id, const DroneAssignment& assignment) {
    for(int pixel_id=0; pixel_id<assignment.size(); pixel_id++) {
      if (assignment[pixel_id] == drone_id) return pixel_id;
    }
    return -1;
  }


};



// -------------------------------------------------------------------------------------------
//   The SPICOMP Simulator
// -------------------------------------------------------------------------------------------

class SpicompSimulator {

  const SpicompSetting& setting;

  const double time_step_duration;
  const int micro_frame_num;

  int sim_step_count;
  int micro_frame_step_count;

  int drone_num;

  GameController game_controller;
  FrameBuffer frame_buffer;

  std::uniform_real_distribution<> rand_scene_x;
  std::uniform_real_distribution<> rand_scene_y;
  std::uniform_real_distribution<> rand_scene_z;

  ContingencyFormationPlan cf_plan;

public:

  explicit SpicompSimulator(const SpicompSetting& setting) :
      setting{setting}, time_step_duration{0.02}, micro_frame_num{5},
      sim_step_count{0}, micro_frame_step_count{0}, drone_num{100},
      game_controller(micro_frame_num), frame_buffer(micro_frame_num),
      rand_scene_x(-setting.getSceneSizeX() / 2.0, setting.getSceneSizeX() / 2.0),
      rand_scene_y(-setting.getSceneSizeY() / 2.0, setting.getSceneSizeY() / 2.0),
      rand_scene_z(0.0, setting.getSceneSizeZ())
  {
    assert(micro_frame_num <= MAX_MICRO_FRAME_NUM);
  }

  void reset();

  void nextStep();

  bool isStopped() const { return false; }


  [[nodiscard]] double getTimeStepDuration() const { return time_step_duration; }

  [[nodiscard]] int getSimStepCount() const { return sim_step_count; }

  [[nodiscard]] Frame getCurrentMicroFrame() const;

private:

  const FormationPlan& getCurrentFormationPlan() const;

};


#endif //SPICOMP_SPICOMP_SIMULATOR_H
