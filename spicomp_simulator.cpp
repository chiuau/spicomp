#include "spicomp_simulator.h"

#include "util/rng.h"
#include "util/stl.h"


void translate(FrameSeq& frame, double x, double y, double z) {
  for(auto& frame : frame) {
    frame.translate(x, y, z);
  }
}


// -------------------------------------------------------------------------------------------
//   Frame Tree
// -------------------------------------------------------------------------------------------

void FrameTree::attachFrameSubtreeToTerminalFrame(const FrameTree& subtree, int subtree_root_frame_id) {
  assert(!subtree.empty());
  assert(isFrameExist(subtree_root_frame_id));
  assert(!isDecisionFrame(subtree_root_frame_id));
  assert(isTerminalFrame(subtree_root_frame_id));  // must be terminal for this function

  if (subtree.isTerminalFrame(subtree_root_frame_id)) return;  // single frame subtree, do nothing

  if (subtree.isDecisionFrame(subtree_root_frame_id)) {
    auto& decision_variable = subtree.getDecisionVariable(subtree_root_frame_id);
    setDecisionVariable(subtree_root_frame_id, decision_variable);

    for(auto [option, child_frame_id] : subtree.getAllChildrenIdsWithOptions(subtree_root_frame_id)) {
      assert(decision_variable.contains(option));
      assert(!isFrameExist(child_frame_id));
      addFrame(subtree.getFrame(child_frame_id));
      addChildId(subtree_root_frame_id, option, child_frame_id);
      attachFrameSubtreeToTerminalFrame(subtree, child_frame_id);
    }
  } else {  // subtree_root_frame is not a decision frame
    // there is only one child frame
    auto& child_frame = subtree.getUniqueChildFrame(subtree_root_frame_id);
    addFrame(child_frame);
    addUniqueChildId(subtree_root_frame_id, child_frame.getId());
    attachFrameSubtreeToTerminalFrame(subtree, child_frame.getId());
  }
}


void FrameTree::attachFrameSubtree(const FrameTree& subtree, int subtree_root_frame_id, const DecisionVariable& new_decision_variable,
                                   DecisionOption option_for_unique_child_in_original, DecisionOption option_for_unique_child_in_subtree)
{
  assert(!subtree.empty());
  assert(isFrameExist(subtree_root_frame_id));
  assert(!isTerminalFrame(subtree_root_frame_id));  // must not be terminal for this function

  if (subtree.isTerminalFrame(subtree_root_frame_id)) return;  // single node subtree, do nothing

  // take care of the frame_decision_var_db and the child_ids in the original tree
  // also check the values in new_decision_variable, option_for_unique_child_in_original and option_for_unique_child_in_subtree
  if (isDecisionFrame(subtree_root_frame_id)) {
    assert(option_for_unique_child_in_original == DecisionVariable::NIL);
    // make sure that old_decision_variable and new_decision_variable are compatible
    auto& old_decision_variable = getDecisionVariable(subtree_root_frame_id);
    assert(old_decision_variable.isSubdomainOf(new_decision_variable));
    if (subtree.isDecisionFrame(subtree_root_frame_id)) {
      assert(option_for_unique_child_in_subtree == DecisionVariable::NIL);
      // make sure that child_decision_variable and new_decision_variable are compatible
      auto& child_decision_variable = subtree.getDecisionVariable(subtree_root_frame_id);
      assert(child_decision_variable.isSubdomainOf(new_decision_variable));
      // make sure no unknown option in new_decision_variable
      for(auto& option : new_decision_variable.getDomains()) {
        assert(old_decision_variable.contains(option) || child_decision_variable.contains(option));
      }
    } else {
      assert(new_decision_variable.contains(option_for_unique_child_in_subtree));
      // make sure no unknown option in new_decision_variable
      for(auto& option : new_decision_variable.getDomains()) {
        assert(old_decision_variable.contains(option) || option == option_for_unique_child_in_subtree);
      }
    }
    // keep children_ids_db unchanged
    // must use the new decision variable
    removeDecisionVariable(subtree_root_frame_id);
    setDecisionVariable(subtree_root_frame_id, new_decision_variable);
  } else {
    assert(option_for_unique_child_in_original != DecisionVariable::NIL);
    assert(new_decision_variable.contains(option_for_unique_child_in_original));
    if (subtree.isDecisionFrame(subtree_root_frame_id)) {
      assert(option_for_unique_child_in_subtree == DecisionVariable::NIL);
      // make sure that child_decision_variable and new_decision_variable are compatible
      auto& child_decision_variable = subtree.getDecisionVariable(subtree_root_frame_id);
      assert(child_decision_variable.isSubdomainOf(new_decision_variable));
      // make sure no unknown option in new_decision_variable
      for(auto& option : new_decision_variable.getDomains()) {
        assert(option == option_for_unique_child_in_original || child_decision_variable.contains(option));
      }
    } else {
      assert(option_for_unique_child_in_subtree != DecisionVariable::NIL);
      assert(new_decision_variable.contains(option_for_unique_child_in_subtree));
      // make sure no unknown option in new_decision_variable
      for(auto& option : new_decision_variable.getDomains()) {
        assert(option == option_for_unique_child_in_original || option == option_for_unique_child_in_subtree);
      }
    }
    // update children_ids_db
    auto& child_frame = getUniqueChildFrame(subtree_root_frame_id);
    addChildId(subtree_root_frame_id, option_for_unique_child_in_original, child_frame.getId());
    // must use the new decision variable
    setDecisionVariable(subtree_root_frame_id, new_decision_variable);
  }

  // now add the child frames in the subtree
  if (option_for_unique_child_in_subtree == DecisionVariable::NIL) {
    auto& child_decision_variable = subtree.getDecisionVariable(subtree_root_frame_id);
    for(auto& option : new_decision_variable.getDomains()) {
      if (child_decision_variable.contains(option)) {
        auto &child_frame = subtree.getChildFrame(subtree_root_frame_id, option);
        assert(!isFrameExist(child_frame.getId()));
        addFrame(child_frame);
        addChildId(subtree_root_frame_id, option, child_frame.getId());
        attachFrameSubtreeToTerminalFrame(subtree, child_frame.getId());  // since child_frame is terminal in this tree.
      }  // else option must be in old_decision_variable -> do nothing
    }
  } else {  // option_for_unique_child_in_subtree != DecisionVariable::NIL
    // there is only one child frame
    auto& child_frame = subtree.getUniqueChildFrame(subtree_root_frame_id);
    assert(!isFrameExist(child_frame.getId()));
    addFrame(child_frame);
    addChildId(subtree_root_frame_id, option_for_unique_child_in_subtree, child_frame.getId());
    attachFrameSubtreeToTerminalFrame(subtree, child_frame.getId());
  }

  assert(isValid());
}


void FrameTree::deleteFrameSubtree(int frame_id) {
  // __vv__(frame_id);
  assert(isFrameExist(frame_id));
  if (root_frame_id == frame_id) {   // delele everything
    clear();
  } else {
    if (isDecisionFrame(frame_id)) {
      if (!isTerminalFrame(frame_id)) {
        auto option_and_child_frame_id = getAllChildrenIdsWithOptions(frame_id);
        for (auto [option, child_frame_id]: option_and_child_frame_id) {
          removeChildId(frame_id, option, child_frame_id);
          deleteFrameSubtree(child_frame_id);
        }
      }
    } else {
      if (!isTerminalFrame(frame_id)) {
        auto child_frame_id = getUniqueChildFrameId(frame_id);
        removeUniqueChildId(frame_id, child_frame_id);
        deleteFrameSubtree(child_frame_id);
      }
    }
    assert(isTerminalFrame(frame_id));  // must have no child
    if (isDecisionFrame(frame_id)) {
      removeDecisionVariable(frame_id);
    }
    removeFrame(frame_id);
  }
}


void FrameTree::pop_front() {
  assert(!empty());
  if (size() == 1) {
    clear(); // just remove everything
  } else {
    if (isDecisionFrame(root_frame_id)) {
      auto& next_frame = getDefaultChildFrame(root_frame_id);
      auto& decision_variable = getDecisionVariable(root_frame_id);
      auto& default_option = getDefaultOption(root_frame_id);
      for(auto& option : getDecisionVariable(root_frame_id).getDomains()) {
        if (option != default_option) {
          auto& ignored_child_frame = getChildFrame(root_frame_id, option);
          removeChildId(root_frame_id, option, ignored_child_frame.getId());  // cut ties with the next frame before delete the subtree.
          deleteFrameSubtree(ignored_child_frame.getId());
        } else {
          removeChildId(root_frame_id, option, next_frame.getId());   // just cut ties with the next frame
        }
      }
      removeDecisionVariable(root_frame_id);
      removeFrame(root_frame_id);
      setRootFrameId(next_frame.getId());
    } else {  // not a decision frame
      auto& next_frame = getUniqueChildFrame(root_frame_id);
      removeUniqueChildId(root_frame_id, next_frame.getId());
      removeFrame(root_frame_id);
      setRootFrameId(next_frame.getId());
    }
  }
  assert(isValid());
}


bool FrameTree::isValid() const {
  std::vector<int> visited_frame_ids;
  if (!isValid(root_frame_id, visited_frame_ids)) return false;
  if (size() != visited_frame_ids.size()) return false;
  for(auto& [frame_id, _] : frame_decision_var_db) {
    if (!isFrameExist(frame_id)) return false;
  }
  for(auto& [frame_id, _] : children_ids_db) {
    if (!isFrameExist(frame_id)) return false;
  }
  for(auto& [frame_id, _] : parent_frame_id_db) {
    if (!isFrameExist(frame_id)) return false;
  }
  for(auto& [frame_id, _] : parent_option_db) {
    if (!isFrameExist(frame_id)) return false;
  }
  return true;
}


bool FrameTree::isValid(int frame_id, std::vector<int>& visited_frame_ids) const {
  if (std::find(visited_frame_ids.begin(), visited_frame_ids.end(), frame_id) != visited_frame_ids.end()) { return false; } // should not visit again
  if (!isFrameExist(frame_id)) return false;

  visited_frame_ids.push_back(frame_id);

  if (hasParentFrameId(frame_id)) {
    if (!parent_option_db.contains(frame_id)) return false;
    auto parent_id = getParentFrameId(frame_id);
    if (isDecisionFrame(parent_id)) {
      if (getChildFrameId(parent_id, getParentOption(frame_id)) != frame_id) return false;
    } else {
      if (getUniqueChildFrameId(parent_id) != frame_id) return false;
    }
  }

  // no need to check frame_decision_var_db

  if (!isTerminalFrame(frame_id)) {
    if (isDecisionFrame(frame_id)) {  // have children
      auto& decision_variable = getDecisionVariable(frame_id);
      for(auto [option, child_id]: getAllChildrenIdsWithOptions(frame_id)) {
        if (option == DecisionVariable::NIL) return false;
        if (!decision_variable.contains(option)) return false;
        if (!isValid(child_id, visited_frame_ids)) return false;
      }
    } else {
      auto& my_children_ids_db = getAllChildrenIdsWithOptions(frame_id);
      if (my_children_ids_db.size() != 1) return false;
      if (!my_children_ids_db.contains(DecisionVariable::NIL)) return false;
      auto child_id = getUniqueChildFrameId(frame_id);
      if (!isValid(child_id, visited_frame_ids)) return false;
    }
  } // else terminal frame, no need to test

  return true;
}


void FrameTree::print() const {
  std::cout << "------ Frame Tree Begin ------" << std::endl;
  if (empty()) {
    assert(root_frame_id < 0);
    std::cout << "Empty frame tree." << std::endl;
  } else {
    assert(root_frame_id >= 0);
    std::cout << "root_frame_id=" << root_frame_id << std::endl;
    print(0, root_frame_id);
  }
  std::cout << "------ Frame Tree End ------" << std::endl;
}


void FrameTree::print(int indent_size, int frame_id) const {
  assert(isFrameExist(frame_id));
  std::cout << indent(indent_size) << "Frame" << frame_id;
  if (isDecisionFrame(frame_id)) {
    assert(!isTerminalFrame(frame_id));
    std::cout << "  " << getDecisionVariable(frame_id);
  } else {
    std::cout << "  NoDV";
  }
  if (hasParentFrameId(frame_id)) {
    int parent_frame_id = getParentFrameId(frame_id);
    assert(parent_frame_id >= 0);
    std::cout << "  Parent=";
    assert(parent_option_db.contains(frame_id));
    if (getParentOption(frame_id) >= 0) {
      std::cout << getParentOption(frame_id) << "->";
      assert(getChildFrameId(parent_frame_id, getParentOption(frame_id)) == frame_id);
    } else {
      // std::cout << "Nil->";
      assert(getUniqueChildFrameId(parent_frame_id) == frame_id);
    }
    std::cout << "Frame" << parent_frame_id;
  } else {
    std::cout << "  NoParent";
  }
  if (isTerminalFrame(frame_id)) {
    assert(!isDecisionFrame(frame_id));
    std::cout << "  Terminal" << std::endl;
  } else {
    std::cout << ":" << std::endl;
    if (isDecisionFrame(frame_id)) {
      for(auto [option , child_id]: getAllChildrenIdsWithOptions(frame_id)) {
        std::cout << indent(indent_size) << "- v" << getDecisionVariable(frame_id).getId() << "=" << option << " in Frame" << frame_id << ":" << std::endl;
        print(indent_size+1, child_id);
      }
    } else {
      assert(hasChildNilOption(frame_id));
      // std::cout << indent(indent_size) << "- Frame" << frame_id << " Nil" << ":" << std::endl;
      print(indent_size+1, getUniqueChildFrame(frame_id).getId());
    }
  }
}


// -------------------------------------------------------------------------------------------
//   Game State
// -------------------------------------------------------------------------------------------

// see https://stackoverflow.com/questions/1197106/static-constructors-in-c-i-need-to-initialize-private-static-objects

const std::vector<Pos3D> GameState::gun_trajectory = [] {
  std::vector<Pos3D> trajectory;

  trajectory.emplace_back(   0.0,-200.0,   0.0);
  trajectory.emplace_back( 200.0,-200.0,   0.0);
  trajectory.emplace_back( 200.0,   0.0,   0.0);
  trajectory.emplace_back( 200.0, 200.0,   0.0);
  trajectory.emplace_back(   0.0, 200.0,   0.0);
  trajectory.emplace_back(-200.0, 200.0,   0.0);
  trajectory.emplace_back(-200.0,   0.0,   0.0);

  trajectory.emplace_back(   0.0,-200.0,   0.0);
  trajectory.emplace_back( 200.0,-200.0,   0.0);
  trajectory.emplace_back( 200.0,   0.0,   0.0);
  trajectory.emplace_back(   0.0, 200.0,   0.0);
  trajectory.emplace_back(-200.0, 200.0,   0.0);
  trajectory.emplace_back(-200.0,   0.0,   0.0);
  trajectory.emplace_back(-200.0,-200.0,   0.0);

  trajectory.emplace_back(   0.0,-200.0,   0.0);
  trajectory.emplace_back( 200.0,-200.0,   0.0);
  trajectory.emplace_back( 200.0,   0.0,   0.0);
  trajectory.emplace_back(   0.0, 200.0,   0.0);
  trajectory.emplace_back(-200.0, 200.0,   0.0);
  trajectory.emplace_back(-200.0,   0.0,   0.0);
  trajectory.emplace_back(-200.0,-200.0,   0.0);

  // TODO: need to fix the dependency with rng.cpp
  std::random_device dev;
  auto rng = std::mt19937{dev()};
  std::uniform_real_distribution<> rand_gen(-100.0, 100.0);

  for(int i=0; i<trajectory.size(); i++) {
    auto dx = rand_gen(rng);
    auto dy = rand_gen(rng);
    trajectory[i].translate(dx, dy, 0.0);
  }

  return trajectory;
}();

//const std::vector<Pos3D> GameState::gun_trajectory = [] {
//  std::vector<Pos3D> trajectory;
//  trajectory.emplace_back(   0.0,-200.0,   0.0);
//  trajectory.emplace_back( 200.0,-200.0,   0.0);
//  trajectory.emplace_back( 200.0,   0.0,   0.0);
//  trajectory.emplace_back( 200.0, 200.0,   0.0);
//  trajectory.emplace_back(   0.0, 200.0,   0.0);
//  trajectory.emplace_back(-200.0,   0.0,   0.0);
//  trajectory.emplace_back(-200.0,-200.0,   0.0);
//  return trajectory;
//}();

//const std::vector<Pos3D> GameState::gun_trajectory = [] {
//  std::vector<Pos3D> trajectory;
//  trajectory.emplace_back(   0.0,   0.0,   0.0);
//  trajectory.emplace_back( 200.0,   0.0,   0.0);
//  trajectory.emplace_back( 200.0, 200.0,   0.0);
//  trajectory.emplace_back(   0.0, 200.0,   0.0);
//  trajectory.emplace_back(   0.0,   0.0,   0.0);
//  trajectory.emplace_back(   0.0,-200.0,   0.0);
//  trajectory.emplace_back(-200.0,-200.0,   0.0);
//  trajectory.emplace_back(-200.0,   0.0,   0.0);
//  return trajectory;
//}();

//const std::vector<Pos3D> GameState::gun_trajectory = [] {
//  std::vector<Pos3D> trajectory;
//  trajectory.emplace_back(  0.0,   0.0,   0.0);
//  trajectory.emplace_back(200.0,   0.0,   0.0);
//  trajectory.emplace_back(200.0, 200.0,   0.0);
//  trajectory.emplace_back(  0.0, 200.0,   0.0);
//  return trajectory;
//}();


//const std::vector<Pixel> GameState::gun_pixels = [] {
//  std::vector<Pixel> result;
//  auto gun_color = COLOR_GREEN;
//
//  result.emplace_back(  0.0,   0.0,  0.0, gun_color);
//  result.emplace_back( 50.0,   0.0,  0.0, gun_color);
//  result.emplace_back(100.0,   0.0,  0.0, gun_color);
//  result.emplace_back(  0.0,  50.0,  0.0, gun_color);
//  result.emplace_back(  0.0, 100.0,  0.0, gun_color);
//  result.emplace_back( 50.0, 100.0,  0.0, gun_color);
//  result.emplace_back(100.0,  50.0,  0.0, gun_color);
//  result.emplace_back(100.0, 100.0,  0.0, gun_color);
//
//  result.emplace_back( 25.0,  25.0, 50.0, gun_color);
//  result.emplace_back( 75.0,  25.0, 50.0, gun_color);
//  result.emplace_back( 25.0,  75.0, 50.0, gun_color);
//  result.emplace_back( 75.0,  75.0, 50.0, gun_color);
//
//  result.emplace_back( 50.0,  50.0, 100.0, gun_color);
//
//  return result;
//};


// -------------------------------------------------------------------------------------------
//   Game State Tree
// -------------------------------------------------------------------------------------------

void GameStateTree::deleteGameStateSubtree(int game_state_id) {
  assert(isGameStateExist(game_state_id));
  if (root_game_state_id == game_state_id) {
    clear();
  } else {
    if (!isTerminalGameState(game_state_id)) {
      for (auto [option, child_frame_id]: getChildrenIds(game_state_id)) {
        deleteGameStateSubtree(child_frame_id);
      }
    }
    game_state_db.erase(game_state_id);
    decision_variable.erase(game_state_id);
    children_ids.erase(game_state_id);
  }
}


void GameStateTree::pop_front() {
  assert(!empty());
  if (size() == 1) {
    clear(); // just remove everything
  } else {
    if (isDecisionGameState(root_game_state_id)) {
      auto& default_option = getDecisionVariable(root_game_state_id).getDefaultOption();
      auto& next_game_state = getChildGameState(root_game_state_id, default_option);
      for(auto& option : getDecisionVariable(root_game_state_id).getDomains()) {
        if (option != default_option) {
          auto& ignored_child_frame = getChildGameState(root_game_state_id, option);
          deleteGameStateSubtree(ignored_child_frame.getId());
        }
      }
      game_state_db.erase(root_game_state_id);
      decision_variable.erase(root_game_state_id);
      children_ids.erase(root_game_state_id);
      root_game_state_id = next_game_state.getId();
    } else {
      auto& next_game_state = getChildGameState(root_game_state_id);
      game_state_db.erase(root_game_state_id);
      assert(!decision_variable.contains(root_game_state_id));
      children_ids.erase(root_game_state_id);
      root_game_state_id = next_game_state.getId();
    }
  }
}


// -------------------------------------------------------------------------------------------
//   The SPICOMP algorithm
// -------------------------------------------------------------------------------------------

bool SpicompPlanner::solve(int frame_id, const Formation& formation, const DroneAssignment& assignment, int search_depth) {
  assert(frame_tree.isFrameExist(frame_id));

  // The depth-first search
  if (frame_tree.isTerminalFrame(frame_id)) { return true; }
  for(auto [option, child_frame_id] : frame_tree.getAllChildrenIdsWithOptions(frame_id)) {
    cf_plan.emplaceFormationPlan(frame_id, child_frame_id);
    auto& fplan = cf_plan.getFormationPlan(frame_id, child_frame_id);
    computeFormationPlan(fplan, frame_tree.getFrame(frame_id), frame_tree.getFrame(child_frame_id), formation, assignment);

    auto& formation2 = fplan.getFormation2();
    auto& assignment2 = fplan.getAssignment2();
    if (!solve(child_frame_id, formation2, assignment2, search_depth+1)) return false;
  }

  return true;
}


bool SpicompPlanner::computeFormationPlan(FormationPlan& fplan, const Frame& frame1, const Frame& frame2, const Formation& formation1, const DroneAssignment& assignment1) {
  assert(frame1.size() == assignment1.size());
  assert(fplan.empty());

  // TODO: should reuse the previous cf_plan

  auto& pixel1_set = frame1.getPixels();
  auto& pixel2_set = frame2.getPixels();

  // initialize formation plan
  for(int frame_id=0; frame_id < micro_frame_num; frame_id++) {
    fplan.addMicroFormation(formation1);      // as a placeholder
  }
  fplan.setFormation1(formation1);
  fplan.setAssignment1(assignment1);

  // assignment to pixel
  assert(frame1.size() >= pixel_trajectory_tracking_num);
  assert(frame2.size() >= pixel_trajectory_tracking_num);
  DroneAssignment assignment2(frame2.size(), -1);
  for(int pixel_id = 0; pixel_id < pixel_trajectory_tracking_num; pixel_id++) {
    assignment2[pixel_id] = assignment1[pixel_id];    // assign to the same drones for trajectory tracking pixels
  }

  fplan.setAssignment2(assignment2);  // set a partial assignment2

  std::unordered_map<int, std::vector<int>> earliest_available_frame_ids_db;

  // for the rest of the pixels:
  if (assignment2.size() > pixel_trajectory_tracking_num) { // deal with the remaining pixels in frame2

    // precompute the list of the earliest available frame ids
    auto unassigned_drone_ids = findUnassignedDroneIds(assignment2);
    for(auto drone_id : unassigned_drone_ids) {
      earliest_available_frame_ids_db[drone_id] = findEarliestAvailableFrameId(fplan, drone_id);
      earliest_available_frame_ids_db[drone_id].push_back(frame2.getId());
    }

    // complete assignment2
    for (int pixel_id = pixel_trajectory_tracking_num; pixel_id < assignment2.size(); pixel_id++) {
      auto& pixel = frame2.getPixel(pixel_id);
      auto selected_iter = findRandomEarliestAvailableDroneId(pixel, formation1, unassigned_drone_ids, earliest_available_frame_ids_db);
      assignment2[pixel_id] = *selected_iter;
      unassigned_drone_ids.erase(selected_iter);
    }
  }
  assert(std::find(assignment2.begin(), assignment2.end(), -1) == assignment2.end());  // check whether every pixel has an assignment.

  fplan.setAssignment2(assignment2);  // set a full assignment2


  // compute the micro formation plans
  for(int pixel2_id = 0; pixel2_id < assignment2.size(); pixel2_id++) {
    int drone_id = assignment2[pixel2_id];
    auto &drone_state = formation1.getDroneState(drone_id);

    auto pixel1 = drone_state.getPixel();
    if (pixel2_id >= pixel_trajectory_tracking_num) {  // hopping pixel
      pixel1 = Pixel(pixel1.getPos(), COLOR_HIDDEN);
    }
    auto pixel2 = pixel2_set.at(pixel2_id);

    if (isDroneAssigned(drone_id, assignment1)) {   // this means that the drone has been used in both assignment1 and assignment2
      computeLinearMicroFormations(fplan, drone_id, pixel1, pixel2);   // we opt for a simple solution
    } else {
      assert(drone_state.getColor() == COLOR_HIDDEN);
      assert(drone_state.getIsHidden());

      // at this point, all trajectory tracking pixels have an assignment.
      computeEarliestAvailableMicroFormations(fplan, drone_id, pixel2, pixel2_id, earliest_available_frame_ids_db.at(drone_id));  // TODO: no need to pass pixel1
      // computeLinearMicroFormations(fplan, drone_id, pixel1, pixel2);   // we opt for a simple solution
    }
  }

  auto unassigned_drone_ids = findUnassignedDroneIds(assignment2);
  for(int drone_id : unassigned_drone_ids) {
    computeGoDarkMicroFormations(fplan, drone_id, formation1.getDroneState(drone_id).getPixel());
  }

  return true;    // TODO: need to check the maximum speed
}


void SpicompPlanner::computeEarliestAvailableMicroFormations(FormationPlan& fplan, int drone_id, const Pixel& pixel2, int pixel2_id, const std::vector<int>& parent_id_list) {
  auto& assignment2 = fplan.getAssignment2();

  assignment2[pixel2_id] = -1;  // MUST temporarily remove the assignment of drone_id in assignment2;
  // auto parent_id_list = findEarliestAvailableFrameId(fplan, drone_id);

  // __vv__(parent_id_list);
  assert(parent_id_list.size() >= 2);
  assert(parent_id_list[parent_id_list.size()-2] == fplan.getFrame1Id());  // since drone_id must not be assigned in assignment1

  int flight_time_step = parent_id_list.size() - 1;


//  // check whether the parent_id_list is correct
//  for(int i=0; i<flight_time_step-1; i++) {
//    auto& tmp_fplan = cf_plan.getFormationPlan(parent_id_list[i], parent_id_list[i+1]);
//    assert(!isDroneAssigned(drone_id, tmp_fplan.getAssignment2()));
//  }

  auto& first_fplan = cf_plan.getFormationPlan(parent_id_list[0], parent_id_list[1]);  // the size of parent_id_list is at least 2
  auto& first_pos = first_fplan.getFormation1().getDroneState(drone_id).getPos();

  auto dist = first_pos.distance(pixel2.getPos());
  auto max_dist = MAX_DRONE_FLIGHT_DISTANCE_PER_FRAME * flight_time_step;
  // __vv__(dist, max_dist);
  assert(dist <= max_dist);  // TODO: need to reduce MAX_DRONE_FLIGHT_DISTANCE_PER_FRAME

  const double MAX_DRONE_FLIGHT_DISTANCE_PER_MICROFRAME = MAX_DRONE_FLIGHT_DISTANCE_PER_FRAME / static_cast<double>(micro_frame_num);

  Pos3D current_pos = first_pos;
  for(int i=0; i<flight_time_step; i++) {
    auto& tmp_fplan = cf_plan.getFormationPlan(parent_id_list[i], parent_id_list[i+1]);
    if (i > 0) {
      tmp_fplan.setFormation1(cf_plan.getFormationPlan(parent_id_list[i-1], parent_id_list[i]).getFormation2());
    }
    auto& tmp_assignment2 = tmp_fplan.getAssignment2();
    assert(!isDroneAssigned(drone_id, tmp_assignment2));
    assert(tmp_fplan.getFormation2().getDroneState(drone_id).getIsHidden());

    for (int micro_frame_id = 0; micro_frame_id < micro_frame_num; micro_frame_id++) {
      auto &next_drone_state = tmp_fplan.getMicroFormation(micro_frame_id).getDroneState(drone_id);

      if (current_pos != pixel2.getPos()) {
        auto dist = current_pos.distance(pixel2.getPos());
        assert(!isZero(dist));
        if (dist > MAX_DRONE_FLIGHT_DISTANCE_PER_MICROFRAME) {
          // just move a distance of MAX_DRONE_FLIGHT_DISTANCE_PER_MICROFRAME
          auto dx = (pixel2.x - current_pos.x) *  MAX_DRONE_FLIGHT_DISTANCE_PER_MICROFRAME / dist;
          auto dy = (pixel2.y - current_pos.y) *  MAX_DRONE_FLIGHT_DISTANCE_PER_MICROFRAME / dist;
          auto dz = (pixel2.z - current_pos.z) *  MAX_DRONE_FLIGHT_DISTANCE_PER_MICROFRAME / dist;
          current_pos.translate(dx, dy, dz);
        } else {  // else can arrive at pixel2 in one microframe
          current_pos = pixel2.getPos();
        }
      }
      next_drone_state.setPos(current_pos);
      auto color = (i == flight_time_step-1 && micro_frame_id == micro_frame_num - 1) ? (pixel2.getColor()) : COLOR_HIDDEN;
      // auto color = (i == flight_time_step-1 && micro_frame_id == micro_frame_num - 1) ? COLOR_BLUE : COLOR_HIDDEN;
      next_drone_state.setColor(color);
    }
    if (i == flight_time_step-2) {
      assert(current_pos == pixel2.getPos());
    }
  }

  assignment2[pixel2_id] = drone_id;  // MUST restore the assignment of drone_id in assignment2;
}


void SpicompPlanner::computeLinearMicroFormations(FormationPlan& fplan, int drone_id, const Pixel& pixel1, const Pixel& pixel2) {
  for (int micro_frame_id = 0; micro_frame_id < micro_frame_num; micro_frame_id++) {
    auto &next_drone_state = fplan.getMicroFormation(micro_frame_id).getDroneState(drone_id);

    // micro_frame_id + 1 ensures that formation1 will not be duplicated.
    auto x = (micro_frame_id == micro_frame_num - 1) ? (pixel2.x) : (pixel1.x + (pixel2.x - pixel1.x) *
                                                                                (static_cast<double>(micro_frame_id + 1) /
                                                                                 static_cast<double>(micro_frame_num)));
    auto y = (micro_frame_id == micro_frame_num - 1) ? (pixel2.y) : (pixel1.y + (pixel2.y - pixel1.y) *
                                                                                (static_cast<double>(micro_frame_id + 1) /
                                                                                 static_cast<double>(micro_frame_num)));
    auto z = (micro_frame_id == micro_frame_num - 1) ? (pixel2.z) : (pixel1.z + (pixel2.z - pixel1.z) *
                                                                                (static_cast<double>(micro_frame_id + 1) /
                                                                                 static_cast<double>(micro_frame_num)));
    auto color = (micro_frame_id == micro_frame_num - 1) ? (pixel2.getColor()) : (pixel1.getColor());

    next_drone_state.setPos(x, y, z);
    next_drone_state.setColor(color);
  }
}


void SpicompPlanner::computeGoDarkMicroFormations(FormationPlan& fplan, int drone_id, const Pixel& pixel1) {
  for (int micro_frame_id = 0; micro_frame_id < micro_frame_num; micro_frame_id++) {
    auto &next_drone_state = fplan.getMicroFormation(micro_frame_id).getDroneState(drone_id);
    // auto color = (micro_frame_id == 0) ? (pixel1.getColor()) : COLOR_HIDDEN;
    auto color = COLOR_HIDDEN;
    next_drone_state.setPos(pixel1.getPos());
    next_drone_state.setColor(color);
  }
}


std::list<int> SpicompPlanner::findUnassignedDroneIds(const std::vector<int>& assignment2) const {
  std::vector<bool> assigned_drone_id(drone_num, false);
  for(int pixel_id=0; pixel_id<assignment2.size(); pixel_id++) {
    auto drone_id = assignment2[pixel_id];
    if (drone_id >= 0) {
      assigned_drone_id[drone_id] = true;
      assert(drone_id < drone_num);
    }
  }
  std::list<int> remaining_drone_ids;
  for(int drone_id=0; drone_id<drone_num; drone_id++) {
    if (!assigned_drone_id[drone_id])  {
      remaining_drone_ids.push_back(drone_id);
    }
  }
  return remaining_drone_ids;
}


std::list<int>::const_iterator SpicompPlanner::findRandomNearbyDroneId(const Pos3D& pixel_pos, const Formation& formation1, const std::list<int>& unassigned_drone_ids) {
  std::vector<double> weights;
  for(auto drone_id : unassigned_drone_ids) {
    auto pos = formation1.getDroneState(drone_id).getPixel().getPos();
    auto distance = pixel_pos.distance(pos);
    weights.push_back(1.0 / (distance + EPSILON));
  }
  auto i = SharedRand::getRandWeightedIndex(weights);
  auto iter = unassigned_drone_ids.cbegin();
  std::advance(iter, i);
  return iter;
}


std::list<int>::const_iterator SpicompPlanner::findRandomEarliestAvailableDroneId(const Pos3D& pixel_pos, const Formation& formation1, const std::list<int>& unassigned_drone_ids, const std::unordered_map<int, std::vector<int>>& earliest_available_frame_ids_db) {
  std::vector<double> weights;
  for(auto drone_id : unassigned_drone_ids) {
    auto pos = formation1.getDroneState(drone_id).getPixel().getPos();
    int flight_time_step = earliest_available_frame_ids_db.at(drone_id).size() - 1;
    auto avg_distance = pixel_pos.distance(pos) / flight_time_step;
    weights.push_back(1.0 / (avg_distance + EPSILON));
  }
  auto i = SharedRand::getRandWeightedIndex(weights);
  auto iter = unassigned_drone_ids.cbegin();
  std::advance(iter, i);
  return iter;
}


void SpicompPlanner::findEarliestAvailableFrameId(std::vector<int>& parent_frame_id_list, const FormationPlan& fplan, int drone_id) const {
  auto& assignment2 = fplan.getAssignment2();
  if (isDroneAssigned(drone_id, assignment2)) {
    return;  // drone_id not available.
  }
  auto frame1_id = fplan.getFrame1Id();
  parent_frame_id_list.push_back(frame1_id);

  if (frame_tree.hasParentFrameId(frame1_id)) {
    auto parent_id = frame_tree.getParentFrameId(frame1_id);
    assert(cf_plan.isFormationPlanExist(parent_id, frame1_id));   // due to DFS, parent formation plan must exist
    auto& parent_fplan = cf_plan.getFormationPlan(parent_id, frame1_id);
    return findEarliestAvailableFrameId(parent_frame_id_list, parent_fplan, drone_id);
  } else {  // no parent frame -> no parent formation plan
    assert(frame1_id == frame_tree.getRootFrameId());
  }
}



// -------------------------------------------------------------------------------------------
//   The SPICOMP Simulator
// -------------------------------------------------------------------------------------------

void SpicompSimulator::reset() {
  sim_step_count = 0;
  micro_frame_step_count = 0;

  game_controller.reset();
  frame_buffer.reset();

  auto init_frame_tree = game_controller.getInitFrameTree();
  assert(!init_frame_tree.empty());
  frame_buffer.setFrameTree(init_frame_tree);

  auto& first_frame = frame_buffer.getFrameTree().getRootFrame();

  assert(drone_num >= first_frame.size());

  // --- initialize the first formation and the first formation plan ---

  Formation current_formation;
  DroneAssignment current_assignment;

  // put drones at the initial frame
  int j=0;
  for(auto& pixel: first_frame.getPixels()) {
    current_formation.addDroneState(pixel);
    current_assignment.push_back(j);
    j++;
  }

  // put the hidden drones at random locations
  auto& rng = SharedRand::getRng();
  for(int i=current_formation.size(); i<drone_num; i++) {
    current_formation.addDroneState(rand_scene_x(rng), rand_scene_y(rng), rand_scene_z(rng));
    // TODO: need to avoid overlapping
  }

  // initialize the current formation plan
  cf_plan.clear();
  SpicompPlanner planner(drone_num, micro_frame_num, frame_buffer.getFrameTree(), current_formation, current_assignment, cf_plan, game_controller.getPixelTrajectoryTrackingNum());
  cf_plan = planner.getContingencyFormationPlan();
}

Frame SpicompSimulator::getCurrentMicroFrame() const {   // this function runs before nextStep()
  auto& fplan = getCurrentFormationPlan();
  return fplan.getMicroFormation(micro_frame_step_count).makeFrame();
}


void SpicompSimulator::nextStep() {
  // __vv__(sim_step_count);
  if (micro_frame_step_count == micro_frame_num-1) {
    auto& fplan = getCurrentFormationPlan();
    auto current_formation = getCurrentFormationPlan().getFormation2();
    auto current_assignment = fplan.getAssignment2();

    game_controller.removeFirstGameState();
    frame_buffer.removeFirstFrame();

     // add new frames to the frame buffer
    auto frame_tree_list = game_controller.getNewFrameTrees();

    if (!frame_tree_list.empty()) {
      // add new frames to the frame buffer
      for(auto& frame_tree: frame_tree_list) {
        frame_buffer.attachFrameTree(frame_tree);
      }
      // update the current formation plan
      SpicompPlanner planner(drone_num, micro_frame_num, frame_buffer.getFrameTree(), current_formation, current_assignment, cf_plan, game_controller.getPixelTrajectoryTrackingNum());
      cf_plan = planner.getContingencyFormationPlan();
    }

    micro_frame_step_count=0;
  } else {
    micro_frame_step_count++;
  }

  game_controller.nextStep();
  frame_buffer.nextStep();
  sim_step_count++;

  assert(game_controller.size() == frame_buffer.size());
}


const FormationPlan& SpicompSimulator::getCurrentFormationPlan() const {
  auto& frame_tree = frame_buffer.getFrameTree();
  // frame_tree.print();
  assert(!frame_tree.empty());
  auto frame_id = frame_tree.getRootFrameId();
  assert(!frame_tree.isTerminalFrame(frame_id));
  int child_frame_id = (frame_tree.isDecisionFrame(frame_id))?(frame_tree.getDefaultChildFrameId(frame_id)):(frame_tree.getUniqueChildFrameId(
      frame_id));

  return cf_plan.getFormationPlan(frame_id, child_frame_id);
}

