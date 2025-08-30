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
    if (ImGui::Begin("telemetry")) {
        updateTelemetry();
        ImGui::End();
    }

    displayECSWorld(m_world);
    updateOperatePanel();
    updateDetailPanel();

    m_node_editor_id.Reset();
}

ecs_world_t* App::GetWorld() {
    return m_world;
}

void App::updateTelemetry() {
    auto& io = ImGui::GetIO();

    ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate,
                io.Framerate ? 1000.0f / io.Framerate : 0.0f);

    ImGui::Separator();

    displayECSWorldByGraph(m_world);
    displayAllTableInSparseSet(m_world);
}

void App::displayECSWorld(ecs_world_t* world) {
    if (ImGui::Begin("world data")) {
        ImGui::SeparatorText("component ids");
        for (int i = 0; i < world->component_ids.count; i++) {
            ImGui::Text("component %" PRId32 ": %" PRId64,
                        *(ecs_id_t*)ecs_vec_get(&world->component_ids,
                                                sizeof(ecs_id_t), i));
        }

        ImGui::SeparatorText("low component records");
        if (ImGui::TreeNode("low component records")) {
            auto component_record_low = world->id_index_lo;
            for (int i = 0; i < FLECS_HI_ID_RECORD_ID; i++) {
                auto component_record = component_record_low[i];
                if (!component_record) {
                    continue;
                }
                displayComponentRecord(component_record,
                                       "component record " + std::to_string(i));
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
                    ecs_component_record_t* component_record =
                        (ecs_component_record_t*)ecs_map_value(&iter);
                    displayComponentRecord(
                        component_record,
                        "hi component " + std::to_string(component_record->id));
                }
            }

            ImGui::TreePop();
        }

        ImGui::End();
    }
}

void App::displayECSWorldByGraph(ecs_world_t* world) {
    ImGui::BeginGroup();
    displayStore(world, &world->store);
    ImGui::EndGroup();
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
        if (m_selected_entity == 0 ||
            !ecs_is_alive(m_world, m_selected_entity)) {
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

    std::array<ecs_id_t, 3> component_ids = {m_id_register->GetPositionID(),
                                             m_id_register->GetNameID(),
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
                    ecs_set_id(m_world, entity, m_id_register->GetPositionID(),
                               sizeof(Position), &p);
                } else if (i == 1) {
                    ecs_add_id(m_world, entity, m_id_register->GetNameID());
                    Name p;
                    p.name = "no-name";
                    ecs_set_id(m_world, entity, m_id_register->GetNameID(),
                               sizeof(Name), &p);
                } else if (i == 2) {
                    ecs_add_id(m_world, entity, m_id_register->GetPlayerID());
                    Player p;
                    ecs_set_id(m_world, entity, m_id_register->GetPlayerID(),
                               sizeof(Player), &p);
                }
            }
        }
        ImGui::EndCombo();
    }
}

void App::displayComponents(ecs_entity_t entity) {
    if (ecs_has_id(m_world, entity, m_id_register->GetPositionID())) {
        Position* position = (Position*)ecs_get_id(
            m_world, entity, m_id_register->GetPositionID());
        displayPositionComponent(*position);
        std::string button_id = "remove##Position" + std::to_string(entity);
        ImGui::SameLine();
        if (ImGui::Button(button_id.c_str())) {
            ecs_remove_id(m_world, entity, m_id_register->GetPositionID());
        }
    }
    if (ecs_has_id(m_world, entity, m_id_register->GetNameID())) {
        Name* name =
            (Name*)ecs_get_id(m_world, entity, m_id_register->GetNameID());
        displayNameComponent(*name);
        std::string button_id = "remove##Name" + std::to_string(entity);
        ImGui::SameLine();
        if (ImGui::Button(button_id.c_str())) {
            ecs_remove_id(m_world, entity, m_id_register->GetNameID());
        }
    }
    if (ecs_has_id(m_world, entity, m_id_register->GetPlayerID())) {
        Player* player =
            (Player*)ecs_get_id(m_world, entity, m_id_register->GetPlayerID());
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

void App::displayComponentRecord(ecs_component_record_t* cr,
                                 std::string label) {
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
}

void App::displaySparseWithTable(ecs_world_t* world, ecs_sparse_t* sparse,
                                 const std::string& label) {
    auto count = ecs_sparse_count(sparse);

    ImGui::Text(label.c_str());

    if (count > 0) {
        /*
        if (ImGui::BeginTable(label.c_str(), count)) {
            for (int i = 0; i < count; i++) {
                uint64_t* dense_elem =
                    ecs_vec_get_t(&sparse->dense, uint64_t, i);
                ImGui::TableSetupColumn(std::to_string(*dense_elem).c_str(),
                                        ImGuiTableColumnFlags_NoSort |
                                            ImGuiTableColumnFlags_NoHide);
            }
            ImGui::TableHeadersRow();
            ImGui::TableNextRow();

            for (int i = 0; i < count; i++) {
                uint64_t* dense_elem =
                    ecs_vec_get_t(&sparse->dense, uint64_t, i);
                ecs_table_t* table =
                    ecs_sparse_get_t(sparse, ecs_table_t, *dense_elem);
            }

            ImGui::EndTable();
        }
        */
    }
}

void App::displayTable(ecs_world_t* world, ecs_table_t* table,
                       bool is_root_table) {
    std::string window_id = "table + " + std::to_string(table->id);
    if (ImGui::Begin(window_id.c_str())) {
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
                        ImGui::TableSetColumnIndex(j + 1);

                        ImGui::Text("unknown");

                        /*

                        ecs_id_t component_id = types.array[i];
                        ecs_column_t* column = table->data.columns + i;
                        if (column && column->data) {
                            void* elem = (char*)column->data + column->ti->size
                        * i; if (elem) { if (component_id ==
                                    m_id_register->GetPositionID()) {
                                    displayPositionComponent(*(Position*)elem);
                                } else if (component_id ==
                                           m_id_register->GetNameID()) {
                                    displayNameComponent(*(Name*)elem);
                                } else if (component_id ==
                                           m_id_register->GetPlayerID()) {
                                    displayPlayerComponent(*(Player*)elem);
                                } else {
                                    auto type_info =
                                        getComponentTypeInfo(component_id);
                                    if (type_info && type_info->name) {
                                        ImGui::Text(type_info->name);
                                    } else {
                                        ImGui::Text("unknown type");
                                    }
                                }
                            } else {
                                ImGui::Text("null");
                            }
                        }
                        */
                    }
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }
}

void App::displayAllTableInSparseSet(ecs_world_t* world) {
    auto& store = world->store;
    auto& sparse = store.tables;
    auto count = ecs_sparse_count(&sparse);
    if (count > 0) {
        for (int i = 0; i < count; i++) {
            uint64_t* dense_elem = ecs_vec_get_t(&sparse.dense, uint64_t, i);
            ecs_table_t* table =
                ecs_sparse_get_t(&sparse, ecs_table_t, *dense_elem);
            bool is_root_table = table == &store.root;
            if (is_root_table) {
                continue;
            }
            displayTable(world, table, is_root_table);
        }
    }
}

const ecs_type_info_t* App::getComponentTypeInfo(ecs_id_t component_id) {
    const ecs_type_info_t* type_info = nullptr;
    if (component_id < FLECS_HI_COMPONENT_ID) {
        auto component_record = m_world->id_index_lo[component_id];
        type_info = component_record->type_info;
    } else {
        ecs_component_record_t* component_record =
            (ecs_component_record_t*)ecs_map_get(&m_world->id_index_hi,
                                                 component_id);
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
