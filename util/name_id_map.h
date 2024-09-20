#ifndef UTIL_NAME_ID_MAP_H
#define UTIL_NAME_ID_MAP_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <stdexcept>

/* ********************************************************************************
 * Forward Declaration for operator<<
 * ******************************************************************************** */


class NameIdMap;

std::ostream& operator<<(std::ostream& out, const NameIdMap& name_id_map);


// -----------------------------------------------------------------------------------------
// A Name-Id Map
// -----------------------------------------------------------------------------------------

class NameIdMap {

  std::vector<std::string> idToNameMap;
  std::unordered_map<std::string,int> nameToIdMap;

public:

  int size() const { return idToNameMap.size(); }

  bool empty() const { return idToNameMap.empty(); }

  const std::vector<std::string>& getNames() const { return idToNameMap; }

  bool hasName(const std::string& name) const {
    return nameToIdMap.contains(name);
  }

  bool hasId(int id) const {
    return 0 <= id && id < idToNameMap.size();
  }

  int getIdByName(const std::string& name) const {   //  return -1 if name not found
    try {
      return nameToIdMap.at(name);
    } catch(const std::out_of_range&) {
      return -1;
    }
  }

  const std::string& getNameById(int id) const {       // return an empty string if id not found
    if (0 <= id && id < idToNameMap.size()) {
      return idToNameMap[id];
    } else {
      return empty_name;
    }
  }

  int addName(const std::string& name) {
    if (name.empty()) throw std::runtime_error("Error in NameIdMap::addName()");
    if (!hasName(name)) {
      idToNameMap.push_back(name);
      int id = idToNameMap.size() - 1;
      nameToIdMap[name] = id;
      return id;
    }  else {   // name exists, no need to assign new id to it
      return nameToIdMap[name];
    }
  }

  int makeNewName(std::string prefix = "x") {
    int i=1;
    while(hasName(prefix + std::to_string(i))) {
      i++;
    }
    std::string new_variable_name = prefix + std::to_string(i);
    return addName(new_variable_name);
  }

  std::string to_string() const {
    std::string s;
    int vid=0;
    for(auto& name : idToNameMap) {
      if (vid != 0) s += " ";
      s += std::to_string(vid) + ":" + name;
      vid++;
    }
    return s;
  }

  friend std::ostream& operator<<(std::ostream& out, const NameIdMap& name_id_map) {
    out << name_id_map.to_string() << std::endl;
    return out;
  }

private:

  static std::string empty_name;
};


#endif //UTIL_NAME_ID_MAP_H
