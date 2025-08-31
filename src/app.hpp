#pragma once
#include "context.hpp"

// clang-format off
#include "flecs.h"
// hack way to include private flecs structures
#include "../flecs/include/flecs/datastructures/bitset.h"
#include "../flecs/src/storage/entity_index.h"
#include "../flecs/src/storage/table_cache.h"
#include "../flecs/src/storage/component_index.h"
#include "../flecs/src/storage/table.h"
#include "../flecs/src/storage/table_graph.h"
#include "../flecs/src/commands.h"
#include "../flecs/src/world.h"
// clang-format on

#include <memory>
#include <unordered_map>
#include <vector>

// component
struct Position {
    float x{}, y{};
};

// component
struct Name {
    std::string name;
};

// tag component
struct Player {};

#define REGISTER_ID(T)                 \
    ecs_id_t Get##T##ID() {            \
        static ecs_id_t id = 0;        \
        if (id == 0) {                 \
            ECS_COMPONENT(m_world, T); \
            id = ecs_id(T);            \
        }                              \
        return id;                     \
    }

class IDRegister {
public:
    explicit IDRegister(ecs_world_t* world) : m_world(world) {}

    REGISTER_ID(Position)
    REGISTER_ID(Name)
    REGISTER_ID(Player)

private:
    ecs_world_t* m_world{};
};

struct ImguiNodeEditorID {
    uint32_t NewID() { return m_id++; }

    void Reset() { m_id = 1; }

private:
    uint32_t m_id{1};
};

class App : public Context {
protected:
    void onInit() override;
    void onQuit() override;
    void onUpdate() override;

    ecs_world_t* GetWorld();

private:
    ecs_world_t* m_world{};
    std::vector<ecs_entity_t> m_entities{};
    ecs_entity_t m_selected_entity = 0;
    std::unique_ptr<IDRegister> m_id_register;
    ImguiNodeEditorID m_node_editor_id;
    std::unordered_map<ecs_table_t*, bool> m_table_open_map;

    void updateTelemetry();
    void displayECSWorld(ecs_world_t*);
    void displayECSWorldByGraph(ecs_world_t*);

    void updateOperatePanel();
    void updateDetailPanel();
    void displayComponentCreateMenu(ecs_entity_t entity);
    void displayComponents(ecs_entity_t entity);
    void displayComponentIDs();

    void displayComponentRecord(ecs_component_record_t*, std::string label);
    void displayStore(ecs_world_t* world, ecs_store_t*);
    void displayTable(ecs_world_t*, ecs_table_t*, bool is_root_table);
    void displayTableMap(ecs_world_t*, ecs_hashmap_t* table_map);
    void displaySparseWithTable(ecs_world_t* world, ecs_sparse_t* sparse, const std::string& label);
    const ecs_type_info_t* getComponentTypeInfo(ecs_id_t);

    void displayPlayerComponent(Player&);
    void displayPositionComponent(Position&);
    void displayNameComponent(Name&);

    void displayGraphEdge(ecs_world_t* world, ecs_graph_edge_t*);
};