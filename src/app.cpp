#include "app.hpp"
#include "imgui.h"

#include <array>
#include <cinttypes>

void App::onInit() {
    m_world = ecs_init();
    m_id_register = std::make_unique<IDRegister>(m_world);
}

void App::onQuit() {
    ecs_fini(m_world);
}

void App::onUpdate() {
    updateTelemetry();

    displayECSWorld(m_world);
    updateOperatePanel();
    updateDetailPanel();

    m_node_editor_id.Reset();
}

ecs_world_t* App::GetWorld() {
    return m_world;
}

void App::updateTelemetry() {
    displayECSWorldByGraph(m_world);
}

void App::displayECSWorld(ecs_world_t* world) {
    if (ImGui::Begin("world data")) {
        ImGui::SeparatorText("component ids");
        for (int i = 0; i < world->component_ids.count; i++) {
            ImGui::Text("component %" PRId32 ": %" PRId64, i,
                        *(ecs_id_t*)ecs_vec_get(&world->component_ids, sizeof(ecs_id_t), i));
        }

        ImGui::SeparatorText("low component records");
        if (ImGui::TreeNode("low component records")) {
            auto component_record_low = world->id_index_lo;
            for (int i = 0; i < FLECS_HI_ID_RECORD_ID; i++) {
                auto component_record = component_record_low[i];
                if (!component_record) {
                    continue;
                }
                displayComponentRecord(component_record, "component record " + std::to_string(i));
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("high component records")) {
            auto component_record_high = world->id_index_hi;
            auto iter = ecs_map_iter(&component_record_high);
            int32_t count = ecs_map_count(&component_record_high);
            ImGui::Text("count: %" PRIu32, count);

            while (ecs_map_next(&iter)) {
                if (iter.bucket) {
                    ecs_component_record_t* component_record = (ecs_component_record_t*)ecs_map_value(&iter);
                    displayComponentRecord(component_record, "hi component " + std::to_string(component_record->id));
                }
            }

            ImGui::TreePop();
        }

        ImGui::End();
    }
}

void App::displayECSWorldByGraph(ecs_world_t* world) {
    displayStore(world, &world->store);
}

void App::updateOperatePanel() {
    if (ImGui::Begin("operator panel")) {
        displayComponentIDs();

        if (ImGui::Button("add entity")) {
            auto entity = ecs_new(m_world);
            m_entities.push_back(entity);
        }
        ImGui::Separator();
        for (int i = 0; i < m_entities.size(); i++) {
            auto entity = m_entities[i];
            std::string name = "Entity " + std::to_string(entity);
            uint32_t flags = ImGuiTreeNodeFlags_Leaf;
            if (entity == m_selected_entity) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }
            if (ImGui::TreeNodeEx(name.c_str(), flags)) {
                if (ImGui::IsItemClicked()) {
                    m_selected_entity = entity;
                }
                ImGui::TreePop();
            }
            ImGui::SameLine();
            std::string button_id = "remove##" + std::to_string(entity);
            if (ImGui::Button(button_id.c_str())) {
                ecs_delete(m_world, entity);
                m_entities.erase(m_entities.begin() + i);
                break;
            }
        }
        ImGui::End();
    }
}

void App::updateDetailPanel() {
    if (ImGui::Begin("detail panel")) {
        if (m_selected_entity == 0 || !ecs_is_alive(m_world, m_selected_entity)) {
            ImGui::End();
            return;
        }

        displayComponentCreateMenu(m_selected_entity);
        ImGui::SeparatorText("components");
        displayComponents(m_selected_entity);

        ImGui::End();
    }
}

void App::displayComponentCreateMenu(ecs_entity_t entity) {
    std::array<const char*, 3> component_names = {
        "Position",
        "Name",
        "Player",
    };

    std::array<ecs_id_t, 3> component_ids = {m_id_register->GetPositionID(), m_id_register->GetNameID(),
                                             m_id_register->GetPlayerID()};

    if (ImGui::BeginCombo("add component", nullptr)) {
        for (int i = 0; i < 3; i++) {
            if (ecs_has_id(m_world, entity, component_ids[i])) {
                continue;
            }

            if (ImGui::Selectable(component_names[i])) {
                if (i == 0) {
                    ecs_add_id(m_world, entity, m_id_register->GetPositionID());
                    Position p;
                    ecs_set_id(m_world, entity, m_id_register->GetPositionID(), sizeof(Position), &p);
                } else if (i == 1) {
                    ecs_add_id(m_world, entity, m_id_register->GetNameID());
                    Name p;
                    p.name = "no-name";
                    ecs_set_id(m_world, entity, m_id_register->GetNameID(), sizeof(Name), &p);
                } else if (i == 2) {
                    ecs_add_id(m_world, entity, m_id_register->GetPlayerID());
                    Player p;
                    ecs_set_id(m_world, entity, m_id_register->GetPlayerID(), sizeof(Player), &p);
                }
            }
        }
        ImGui::EndCombo();
    }
}

void App::displayComponents(ecs_entity_t entity) {
    if (ecs_has_id(m_world, entity, m_id_register->GetPositionID())) {
        Position* position = (Position*)ecs_get_id(m_world, entity, m_id_register->GetPositionID());
        displayPositionComponent(*position);
        std::string button_id = "remove##Position" + std::to_string(entity);
        ImGui::SameLine();
        if (ImGui::Button(button_id.c_str())) {
            ecs_remove_id(m_world, entity, m_id_register->GetPositionID());
        }
    }
    if (ecs_has_id(m_world, entity, m_id_register->GetNameID())) {
        Name* name = (Name*)ecs_get_id(m_world, entity, m_id_register->GetNameID());
        displayNameComponent(*name);
        std::string button_id = "remove##Name" + std::to_string(entity);
        ImGui::SameLine();
        if (ImGui::Button(button_id.c_str())) {
            ecs_remove_id(m_world, entity, m_id_register->GetNameID());
        }
    }
    if (ecs_has_id(m_world, entity, m_id_register->GetPlayerID())) {
        Player* player = (Player*)ecs_get_id(m_world, entity, m_id_register->GetPlayerID());
        displayPlayerComponent(*player);
        std::string button_id = "remove##Player" + std::to_string(entity);
        ImGui::SameLine();
        if (ImGui::Button(button_id.c_str())) {
            ecs_remove_id(m_world, entity, m_id_register->GetPlayerID());
        }
    }
}

void App::displayComponentIDs() {
    ImGui::SeparatorText("component ids");
    ImGui::Text("Position: %" PRIu64, m_id_register->GetPositionID());
    ImGui::Text("Player: %" PRIu64, m_id_register->GetPlayerID());
    ImGui::Text("Name: %" PRIu64, m_id_register->GetNameID());
}

void App::displayComponentRecord(ecs_component_record_t* cr, std::string label) {
    if (!cr) {
        return;
    }

    if (cr->type_info && cr->type_info->name) {
        label += ": " + std::string{cr->type_info->name};
    }
    if (ImGui::TreeNodeEx(label.c_str())) {
        ImGui::Text("id: %" PRIu64, cr->id);
        ImGui::Text("keep alive: %" PRId32, cr->keep_alive);
        ImGui::TreePop();
    }
}

void App::displayStore(ecs_world_t* world, ecs_store_t* store) {
    displayTable(world, &store->root, true);
    displaySparseWithTable(world, &store->tables, "sparse set");
    displayTableMap(world, &store->table_map);
}

void App::displaySparseWithTable(ecs_world_t* world, ecs_sparse_t* sparse, const std::string& label) {
    auto count = ecs_sparse_count(sparse);

    ImGui::Text("%s", label.c_str());

    if (count > 0) {
        auto& store = world->store;
        auto& sparse = store.tables;
        auto count = ecs_sparse_count(&sparse);
        if (count > 0) {
            for (int i = 1; i <= count; i++) {
                uint64_t* dense_elem = ecs_vec_get_t(&sparse.dense, uint64_t, i);
                ecs_table_t* table = ecs_sparse_get_t(&sparse, ecs_table_t, *dense_elem);
                bool is_root_table = table == &store.root;
                if (is_root_table) {
                    continue;
                }
                displayTable(world, table, is_root_table);
            }
        }
    }
}

void App::displayTable(ecs_world_t* world, ecs_table_t* table, bool is_root_table) {
    // only draw tables contain our own components
    for (int j = 0; j < table->type.count; j++) {
        ecs_id_t component_id = table->type.array[j];
        if (!(component_id == m_id_register->GetNameID() || component_id == m_id_register->GetPlayerID() ||
              component_id == m_id_register->GetPositionID())) {
            return;
        }
    }

    std::string window_id = "table + " + std::to_string(table->id);
    if (is_root_table) {
        window_id = "root table";
    }

    bool& open = m_table_open_map.emplace(table, true).first->second;

    if (open && ImGui::Begin(window_id.c_str(), &open)) {
        if (is_root_table) {
            ImGui::Text("root table");
        } else {
            ImGui::Text("table id: %" PRIu64, table->id);
        }
        ImGui::Separator();
        std::string table_id = "content##" + std::to_string(table->id);

        if (table->column_count == 0) {
            auto& data = table->data;
            for (int i = 0; i < data.count; i++) {
                ecs_entity_t entity = data.entities[i];
                ImGui::Text("entity %" PRIu64, entity);
            }
        } else {
            auto& types = table->type;
            if (ImGui::BeginTable(table_id.c_str(), types.count + 1)) {
                ImGui::TableSetupColumn("component/entity");
                for (int i = 0; i < types.count; i++) {
                    ecs_id_t component_id = types.array[i];
                    auto type_info = getComponentTypeInfo(component_id);

                    std::string column_head_name = "unknown type";
                    if (type_info && type_info->name) {
                        column_head_name = type_info->name;
                    }
                    ImGui::TableSetupColumn(column_head_name.c_str());
                }

                ImGui::TableHeadersRow();

                for (int i = 0; i < table->data.count; i++) {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("entity %" PRIu64, table->data.entities[i]);

                    for (int j = 0; j < types.count; j++) {
                        ecs_id_t component_id = types.array[j];
                        ImGui::TableSetColumnIndex(j + 1);
                        void* elem = nullptr;
                        if (component_id >= FLECS_HI_COMPONENT_ID) {
                            ecs_component_record_t* cr = flecs_components_get(world, component_id);
                            if (cr->flags & (EcsIdIsSparse | EcsIdDontFragment)) {
                                ImGui::Text("in sparse");
                            } else {
                                auto& cache = cr->cache;
                                if (ecs_map_is_init(&cache.index)) {
                                    elem = ecs_map_get_deref(&cache.index, void**, table->id);
                                }
                            }
                        } else {
                            int16_t column_index = table->component_map[component_id];
                            if (column_index > 0) {
                                ecs_column_t* column = &table->data.columns[column_index - 1];
                                elem = ECS_ELEM(column->data, column->ti->size, ECS_RECORD_TO_ROW(i));

                            } else if (column_index < 0) {
                                ImGui::Text("column index < 0, unhandled");
                            }
                        }

                        if (elem) {
                            if (component_id == m_id_register->GetPositionID()) {
                                displayPositionComponent(*(Position*)elem);
                            } else if (component_id == m_id_register->GetNameID()) {
                                displayNameComponent(*(Name*)elem);
                            } else if (component_id == m_id_register->GetPlayerID()) {
                                displayPlayerComponent(*(Player*)elem);
                            } else {
                                auto type_info = getComponentTypeInfo(component_id);
                                if (type_info && type_info->name) {
                                    ImGui::Text("%s", type_info->name);
                                } else {
                                    ImGui::Text("unknown type");
                                }
                            }
                        } else {
                            ImGui::Text("null");
                        }
                    }
                }
                ImGui::EndTable();
            }
        }

        ImGui::SeparatorText("edges");
        if (ImGui::TreeNode("add edge")) {
            if (table->node.add.lo) {
                if (ImGui::TreeNode("low edges")) {
                    for (int i = 0; i < FLECS_HI_COMPONENT_ID; i++) {
                        std::string node_id = "add node " + std::to_string(i);
                        auto& edge = table->node.add.lo[i];
                        if (edge.id == 0) {
                            continue;
                        }
                        displayGraphEdge(world, &edge);
                    }
                    ImGui::TreePop();
                }
            }
            if (table->node.add.hi) {
                if (ImGui::TreeNode("high edges")) {
                    auto iter = ecs_map_iter(table->node.add.hi);
                    while (ecs_map_next(&iter)) {
                        ecs_graph_edge_t* edge = (ecs_graph_edge_t*)ecs_map_value(&iter);
                        displayGraphEdge(world, edge);
                    }
                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("remove edges")) {
            if (table->node.remove.lo) {
                if (ImGui::TreeNode("low edges")) {
                    for (int i = 0; i < FLECS_HI_COMPONENT_ID; i++) {
                        std::string node_id = "remove node " + std::to_string(i);
                        auto edge = table->node.remove.lo[i];
                        if (edge.id == 0) {
                            continue;
                        }
                        displayGraphEdge(world, &edge);
                    }
                    ImGui::TreePop();
                }
            }
            if (table->node.remove.hi) {
                if (ImGui::TreeNode("high edges")) {
                    auto iter = ecs_map_iter(table->node.remove.hi);
                    while (ecs_map_next(&iter)) {
                        ecs_graph_edge_t* edge = (ecs_graph_edge_t*)ecs_map_value(&iter);
                        displayGraphEdge(world, edge);
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        ImGui::End();
    }
}

void App::displayTableMap(ecs_world_t*, ecs_hashmap_t* table_map) {
    flecs_hashmap_iter_t iter = flecs_hashmap_iter(table_map);
}

const ecs_type_info_t* App::getComponentTypeInfo(ecs_id_t component_id) {
    const ecs_type_info_t* type_info = nullptr;
    if (component_id < FLECS_HI_COMPONENT_ID) {
        auto component_record = m_world->id_index_lo[component_id];
        type_info = component_record->type_info;
    } else {
        ecs_component_record_t* component_record =
            (ecs_component_record_t*)ecs_map_get(&m_world->id_index_hi, component_id);
        if (component_record) {
            type_info = component_record->type_info;
        }
    }

    return type_info;
}

void App::displayPlayerComponent(Player&) {
    ImGui::LabelText("Player", "");
}

void App::displayPositionComponent(Position& position) {
    ImGui::DragFloat2("position", (float*)&position, 0.1);
}

void App::displayNameComponent(Name& name) {
    char buf[1024] = {0};
    strcpy(buf, name.name.c_str());
    ImGui::InputText("name", buf, sizeof(buf));
    name.name = buf;
}

void App::displayGraphEdge(ecs_world_t* world, ecs_graph_edge_t* edge) {
    if (!edge) {
        return;
    }

    auto id = edge->id;
    std::string node_id = "unknown component";
    if (id != 0) {
        auto type_info = getComponentTypeInfo(id);
        if (type_info && type_info->name) {
            node_id = type_info->name;
        }
    }

    if (ImGui::TreeNode(node_id.c_str())) {
        if (edge->from) {
            if (edge->from == &world->store.root) {
                ImGui::Text("from table: root");
            } else {
                ImGui::Text("from table: table %" PRIu64, edge->from->id);
            }
        }

        if (edge->to) {
            if (edge->to == &world->store.root) {
                ImGui::Text("to table: root");
            } else {
                ImGui::Text("to table: table %" PRIu64, edge->to->id);
            }
        }

        if (edge->diff) {
            std::string diff_id = "diff##" + std::to_string((intptr_t)edge);
            if (ImGui::TreeNode(diff_id.c_str())) {
                std::string diff_add_id = "add##" + std::to_string((intptr_t)edge);
                if (ImGui::TreeNode(diff_add_id.c_str())) {
                    for (int i = 0; i < edge->diff->added.count; i++) {
                        ecs_id_t id = edge->diff->added.array[i];
                        const ecs_type_info_t* type_info = getComponentTypeInfo(id);
                        ImGui::Text("%s", (type_info && type_info->name) ? type_info->name : "unknown");
                    }
                    ImGui::TreePop();
                }
                std::string diff_remove_id = "remove##" + std::to_string((intptr_t)edge);
                if (ImGui::TreeNode(diff_remove_id.c_str())) {
                    for (int i = 0; i < edge->diff->removed.count; i++) {
                        ecs_id_t id = edge->diff->removed.array[i];
                        const ecs_type_info_t* type_info = getComponentTypeInfo(id);
                        ImGui::Text("%s", (type_info && type_info->name) ? type_info->name : "unknown");
                    }
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }
}
