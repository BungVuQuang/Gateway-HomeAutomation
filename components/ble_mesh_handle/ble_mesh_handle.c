/**
 ******************************************************************************
 * @file		ble_mesh.c
 * @author	Vu Quang Bung
 * @date		26 June 2023
 * Copyright (C) 2023 - Bung.VQ
 ******************************************************************************
 **/
/*-----------------------------------------------------------------------------*/
/* Header inclusions */
/*-----------------------------------------------------------------------------*/
#include "ble_mesh_handle.h"
// #include "ble_mesh_example_init.h"
// #include "ble_mesh_example_nvs.h"
/*-----------------------------------------------------------------------------*/
/* Local Constant definitions */
/*-----------------------------------------------------------------------------*/
const char *TAG_BLE = "ble_mesh";
const char *NVS_KEY_MESH_SIZE = "DEVICE";
/*-----------------------------------------------------------------------------*/
/* Local Macro definitions */
/*-----------------------------------------------------------------------------*/
#define APP_KEY_IDX 0x0000
#define APP_KEY_OCTET 0x12

#define COMP_DATA_1_OCTET(msg, offset) (msg[offset])
#define COMP_DATA_2_OCTET(msg, offset) (msg[offset + 1] << 8 | msg[offset])

#define ESP_BLE_MESH_VND_MODEL_ID_CLIENT 0x0000
#define ESP_BLE_MESH_VND_MODEL_ID_SERVER 0x0001
#define ESP_BLE_MESH_VND_MODEL_ID_TEMPERATURE_CLIENT 0x0002

#define PROV_OWN_ADDR 0x0001
#define SUBSCRIPTION_ADDR 0xC000
#define MSG_SEND_TTL 3
#define MSG_SEND_REL false
#define MSG_TIMEOUT 0
#define MSG_ROLE ROLE_PROVISIONER
#define COMP_DATA_PAGE_0 0x00
#define SL_BTMESH_VENDOR_MESSAGE_STATUS_ID 0x01
/*-----------------------------------------------------------------------------*/
/* Local Data type definitions */
/*-----------------------------------------------------------------------------*/
uint16_t server_addr; /* Vendor server unicast address */
/*-----------------------------------------------------------------------------*/
/* Global variables */
/*-----------------------------------------------------------------------------*/
nvs_handle_t NVS_HANDLE;
extern nvs_handle_t my_handler;
extern char data_tx_ble[20];
uint8_t dev_uuid[ESP_BLE_MESH_OCTET16_LEN];
uint8_t rx_node = 0;
uint8_t rx_mess_check = 0;
const char *NVS_KEY = "Node1";
uint8_t mqtt_check = 0;
char data_ack[5] = "ACK";
int unicast_delete;
char data_mesh_rx[80];
char data_mesh_info[150];
char role[10];
char unicast[10];
char mac[30];
uint16_t provision_addr = 0;

static esp_ble_mesh_client_t sensor_client;
static esp_ble_mesh_cfg_srv_t config_server = {
    .relay = ESP_BLE_MESH_RELAY_ENABLED,
    .beacon = ESP_BLE_MESH_BEACON_ENABLED,
#if defined(CONFIG_BLE_MESH_FRIEND)
    .friend_state = ESP_BLE_MESH_FRIEND_ENABLED,
#else
    .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
#endif
    .default_ttl = 7,
    /* 3 transmissions with 20ms interval */
    .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
};

static esp_ble_mesh_client_t config_client;

static const esp_ble_mesh_client_op_pair_t vnd_op_pair[] = {
    {ESP_BLE_MESH_VND_MODEL_OP_SEND, ESP_BLE_MESH_VND_MODEL_OP_STATUS},
};
ESP_BLE_MESH_MODEL_PUB_DEFINE(vendor_pub, 8, ROLE_PROVISIONER);
static esp_ble_mesh_client_t vendor_client = {
    .op_pair_size = ARRAY_SIZE(vnd_op_pair),
    .op_pair = vnd_op_pair,
};
uint8_t temp_pro = 1;

static esp_ble_mesh_model_op_t vnd_op[] = {
    ESP_BLE_MESH_MODEL_OP(ESP_BLE_MESH_VND_MODEL_OP_STATUS, 2),
    ESP_BLE_MESH_MODEL_OP_END,
};

// ESP_BLE_MESH_MODEL_PUB_DEFINE(onoff_pub_0, 2 + 3, ROLE_NODE);
// static esp_ble_mesh_gen_onoff_srv_t onoff_server_0 = {
//     .rsp_ctrl.get_auto_rsp = ESP_BLE_MESH_SERVER_AUTO_RSP,
//     .rsp_ctrl.set_auto_rsp = ESP_BLE_MESH_SERVER_AUTO_RSP,
// };

static esp_ble_mesh_model_t root_models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
    ESP_BLE_MESH_MODEL_CFG_CLI(&config_client),
    ESP_BLE_MESH_MODEL_SENSOR_CLI(NULL, &sensor_client),
};
//&vendor_pub
static esp_ble_mesh_model_t vnd_models[] = {
    ESP_BLE_MESH_VENDOR_MODEL(CID_ESP, ESP_BLE_MESH_VND_MODEL_ID_CLIENT,
                              vnd_op, NULL, &vendor_client),
};

static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, root_models, vnd_models),
};

static esp_ble_mesh_comp_t composition = {
    .cid = CID_ESP,
    .elements = elements,
    .element_count = ARRAY_SIZE(elements),
};

static struct esp_ble_mesh_key
{
    uint16_t net_idx;
    uint16_t app_idx;
    uint8_t app_key[ESP_BLE_MESH_OCTET16_LEN];
} prov_key;

static esp_ble_mesh_prov_t provision = {
    .prov_uuid = dev_uuid,
    .prov_unicast_addr = PROV_OWN_ADDR,
    .prov_start_address = 0x0005,
    // .prov_uuid = dev_uuid,
    // .prov_unicast_addr = PROV_OWN_ADDR,
    // .prov_start_address = 0x0005,
    // .prov_attention = 0x00,
    // .prov_algorithm = 0x00,
    // .prov_pub_key_oob = 0x00,
    // .prov_static_oob_val = NULL,
    // .prov_static_oob_len = 0x00,
    // .flags = 0x00,
    // .iv_index = 0x00,
};

/*========================HEART*/
static const char *TAG = "app_ble_mesh";
typedef struct
{
    uint16_t unicast;
    uint8_t count;
} ble_mesh_node_hb_t;
#define SUBSCRIBE_DEST 0xC001
static ble_mesh_node_hb_t node_hb[CONFIG_BLE_MESH_MAX_PROV_NODES] = {0};

/*-----------------------------------------------------------------------------*/
/* Function prototypes */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Function implementations */
/*-----------------------------------------------------------------------------*/

/*======================================HEART BREATH=======================*/

static void ble_mesh_recv_hb(uint16_t hb_src)
{
    /* Judge if the device has been added before */
    for (int i = 0; i < ARRAY_SIZE(node_hb); i++)
    {
        if (node_hb[i].unicast == hb_src)
        {
            node_hb[i].count++;
            return;
        }
    }

    for (int j = 0; j < ARRAY_SIZE(node_hb); j++)
    {
        if (node_hb[j].unicast == ESP_BLE_MESH_ADDR_UNASSIGNED)
        {
            node_hb[j].unicast = hb_src;
            node_hb[j].count = 1;
            return;
        }
    }
}

/*======================================END HEART BREATH=======================*/
/**
 *  @brief Hàm này thực hiện xoá 1 node ra khỏi mạng
 *
 *  @param[in] addr Unicast address
 *  @return none
 */
void Provision_Handler(int addr)
{
    printf("Da xoa node %d\n", addr);
    esp_ble_mesh_provisioner_delete_node_with_addr(addr);
}

static void example_ble_mesh_set_msg_common(esp_ble_mesh_client_common_param_t *common,
                                            esp_ble_mesh_node_t *node,
                                            esp_ble_mesh_model_t *model, uint32_t opcode)
{
    common->opcode = opcode;
    common->model = model;
    common->ctx.net_idx = prov_key.net_idx;
    common->ctx.app_idx = prov_key.app_idx;
    common->ctx.addr = node->unicast_addr;
    common->ctx.send_ttl = MSG_SEND_TTL;
    common->ctx.send_rel = MSG_SEND_REL;
    common->msg_timeout = MSG_TIMEOUT;
    common->msg_role = MSG_ROLE;
}

/**
 *  @brief Hàm này thực hiện sau khi hoàn thành việc cấp phép
 *
 *  @param[in] node_index index của node trong mạng
 *  @param[in] uuid UUID
 *  @param[in] primary_addr Unicast address
 *  @param[in] element_num element_num
 *  @param[in] net_idx net_idx
 *  @return none
 */
static esp_err_t prov_complete(uint16_t node_index, const esp_ble_mesh_octet16_t uuid,
                               uint16_t primary_addr, uint8_t element_num, uint16_t net_idx)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_cfg_client_get_state_t get = {0};
    esp_ble_mesh_node_t *node = NULL;
    char name[10] = {'\0'};
    esp_err_t err;

    ESP_LOGI(TAG_BLE, "node_index %u, primary_addr 0x%04x, element_num %u, net_idx 0x%03x",
             node_index, primary_addr, element_num, net_idx);
    ESP_LOG_BUFFER_HEX("uuid", uuid, ESP_BLE_MESH_OCTET16_LEN);
    server_addr = primary_addr;

    sprintf(name, "%s%02x", "NODE-", node_index);
    err = esp_ble_mesh_provisioner_set_node_name(node_index, name);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_BLE, "Failed to set node name");
        return ESP_FAIL;
    }

    node = esp_ble_mesh_provisioner_get_node_with_addr(primary_addr);
    if (node == NULL)
    {
        ESP_LOGE(TAG_BLE, "Failed to get node 0x%04x info", primary_addr);
        return ESP_FAIL;
    }

    example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET);
    get.comp_data_get.page = COMP_DATA_PAGE_0;
    err = esp_ble_mesh_config_client_get_state(&common, &get);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_BLE, "Failed to send Config Composition Data Get");
        return ESP_FAIL;
    }
    sprintf(role, "Node%d", primary_addr - 4);
    sprintf(unicast, "0x%04x", primary_addr);
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7]);
    sprintf(data_mesh_info, "{\"%s\":{\"role\": \"%s\",\"parent\": \"No\",\"uuid\": \"%s\",\"unicast\": \"%s\"}}", role, role, mac, unicast);
    meshSize = meshSize + 1;
    nvs_save_Info(my_handler, NVS_KEY_MESH_SIZE, &meshSize, sizeof(int));
    return ESP_OK;
}

/**
 *  @brief Hàm này thực hiện gửi yêu cầu cấp phếp đến node nếu đúng định dạng UUID
 *
 *  @param[in] dev_uuid uuid của node
 *  @param[in] addr unicast address của node
 *  @param[in] addr_type addr_type
 *  @param[in] oob_info oob_info
 *  @param[in] adv_type adv_type
 *  @param[in] bearer loại bearer
 *  @return none
 */
static void recv_unprov_adv_pkt(uint8_t dev_uuid[ESP_BLE_MESH_OCTET16_LEN], uint8_t addr[BD_ADDR_LEN],
                                esp_ble_mesh_addr_type_t addr_type, uint16_t oob_info,
                                uint8_t adv_type, esp_ble_mesh_prov_bearer_t bearer)
{
    esp_ble_mesh_unprov_dev_add_t add_dev = {0};
    esp_err_t err;
    ESP_LOG_BUFFER_HEX("Device address", addr, BD_ADDR_LEN);
    ESP_LOGI(TAG_BLE, "Address type 0x%02x, adv type 0x%02x", addr_type, adv_type);
    ESP_LOG_BUFFER_HEX("Device UUID", dev_uuid, ESP_BLE_MESH_OCTET16_LEN);
    ESP_LOGI(TAG_BLE, "oob info 0x%04x, bearer %s", oob_info, (bearer & ESP_BLE_MESH_PROV_ADV) ? "PB-ADV" : "PB-GATT");

    memcpy(add_dev.addr, addr, BD_ADDR_LEN);
    add_dev.addr_type = (uint8_t)addr_type;
    memcpy(add_dev.uuid, dev_uuid, ESP_BLE_MESH_OCTET16_LEN);
    add_dev.oob_info = oob_info;
    add_dev.bearer = (uint8_t)bearer;
    /* Note: If unprovisioned device adv packets have not been received, we should not add
             device with ADD_DEV_START_PROV_NOW_FLAG set. */

    err = esp_ble_mesh_provisioner_add_unprov_dev(&add_dev,
                                                  ADD_DEV_RM_AFTER_PROV_FLAG | ADD_DEV_START_PROV_NOW_FLAG | ADD_DEV_FLUSHABLE_DEV_FLAG);
    // err = esp_ble_mesh_provisioner_add_unprov_dev(&add_dev,
    //                                               ADD_DEV_FLUSHABLE_DEV_FLAG);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_BLE, "Failed to start provisioning device");
    }
}

/**
 *  @brief Hàm này gọi lại khi có event cấp phép đến 1 node
 *
 *  @param[in] event Event của callBack
 *  @param[in] param Thông tin của Node trong gói quảng cáo
 *  @return none
 */
static void example_ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                                             esp_ble_mesh_prov_cb_param_t *param)
{
    switch (event)
    {
    case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
        ESP_LOGI(TAG_BLE, "ESP_BLE_MESH_PROV_REGISTER_COMP_EVT, err_code %d", param->prov_register_comp.err_code);
        // mesh_example_info_restore(); /* Restore proper mesh example info */
        break;
    case ESP_BLE_MESH_PROVISIONER_PROV_ENABLE_COMP_EVT: // sự kiện bật scanner các gói tin quảng cáo
        ESP_LOGI(TAG_BLE, "ESP_BLE_MESH_PROVISIONER_PROV_ENABLE_COMP_EVT, err_code %d", param->provisioner_prov_enable_comp.err_code);
        break;
    case ESP_BLE_MESH_PROVISIONER_PROV_DISABLE_COMP_EVT: // sự kiện tắt scanner các gói tin quảng cáo
        ESP_LOGI(TAG_BLE, "ESP_BLE_MESH_PROVISIONER_PROV_DISABLE_COMP_EVT, err_code %d", param->provisioner_prov_disable_comp.err_code);
        break;
    case ESP_BLE_MESH_PROVISIONER_RECV_UNPROV_ADV_PKT_EVT: // sự kiện phát hiện thiết bị chưa đc cấp phép
        // esp_wifi_stop();
        // TaskHandle_t xTaskRtcHandle;
        // StaticTask_t xTaskWsHandle;
        // StaticTask_t xTaskNetworkHandle;
        // vTaskSuspend(xTaskRtcHandle);
        // vTaskSuspend(xTaskWsHandle);
        //  vTaskSuspend(xTaskNetworkHandle);
        ESP_LOGI(TAG_BLE, "ESP_BLE_MESH_PROVISIONER_RECV_UNPROV_ADV_PKT_EVT");
        recv_unprov_adv_pkt(param->provisioner_recv_unprov_adv_pkt.dev_uuid, param->provisioner_recv_unprov_adv_pkt.addr,
                            param->provisioner_recv_unprov_adv_pkt.addr_type, param->provisioner_recv_unprov_adv_pkt.oob_info,
                            param->provisioner_recv_unprov_adv_pkt.adv_type, param->provisioner_recv_unprov_adv_pkt.bearer);
        break;
    case ESP_BLE_MESH_PROVISIONER_PROV_LINK_OPEN_EVT:
        ESP_LOGI(TAG_BLE, "ESP_BLE_MESH_PROVISIONER_PROV_LINK_OPEN_EVT, bearer %s",
                 param->provisioner_prov_link_open.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_PROVISIONER_PROV_LINK_CLOSE_EVT:
        ESP_LOGI(TAG_BLE, "ESP_BLE_MESH_PROVISIONER_PROV_LINK_CLOSE_EVT, bearer %s, reason 0x%02x",
                 param->provisioner_prov_link_close.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT", param->provisioner_prov_link_close.reason);
        break;
    case ESP_BLE_MESH_PROVISIONER_PROV_COMPLETE_EVT: // sự kiện hoàn thành cấp phép
        prov_complete(param->provisioner_prov_complete.node_idx, param->provisioner_prov_complete.device_uuid,
                      param->provisioner_prov_complete.unicast_addr, param->provisioner_prov_complete.element_num,
                      param->provisioner_prov_complete.netkey_idx);
        break;
    case ESP_BLE_MESH_PROVISIONER_ADD_UNPROV_DEV_COMP_EVT:
        ESP_LOGI(TAG_BLE, "ESP_BLE_MESH_PROVISIONER_ADD_UNPROV_DEV_COMP_EVT, err_code %d", param->provisioner_add_unprov_dev_comp.err_code);
        break;
    case ESP_BLE_MESH_PROVISIONER_SET_DEV_UUID_MATCH_COMP_EVT: // match UUID của mode
        ESP_LOGI(TAG_BLE, "ESP_BLE_MESH_PROVISIONER_SET_DEV_UUID_MATCH_COMP_EVT, err_code %d", param->provisioner_set_dev_uuid_match_comp.err_code);
        break;
    case ESP_BLE_MESH_PROVISIONER_SET_NODE_NAME_COMP_EVT:
        ESP_LOGI(TAG_BLE, "ESP_BLE_MESH_PROVISIONER_SET_NODE_NAME_COMP_EVT, err_code %d", param->provisioner_set_node_name_comp.err_code);
        if (param->provisioner_set_node_name_comp.err_code == 0)
        {
            const char *name = esp_ble_mesh_provisioner_get_node_name(param->provisioner_set_node_name_comp.node_index);
            if (name)
            {
                ESP_LOGI(TAG_BLE, "Node %d name %s", param->provisioner_set_node_name_comp.node_index, name);
            }
        }
        break;
    case ESP_BLE_MESH_PROVISIONER_ADD_LOCAL_APP_KEY_COMP_EVT: // add app key vào các model
        ESP_LOGI(TAG_BLE, "ESP_BLE_MESH_PROVISIONER_ADD_LOCAL_APP_KEY_COMP_EVT, err_code %d", param->provisioner_add_app_key_comp.err_code);
        if (param->provisioner_add_app_key_comp.err_code == 0)
        {
            esp_err_t err;
            prov_key.app_idx = param->provisioner_add_app_key_comp.app_idx;
            // add appkey vào sensor model
            err = esp_ble_mesh_provisioner_bind_app_key_to_local_model(PROV_OWN_ADDR, prov_key.app_idx,
                                                                       ESP_BLE_MESH_MODEL_ID_SENSOR_CLI, ESP_BLE_MESH_CID_NVAL);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG_BLE, "Failed to bind AppKey to sensor client");
            }
            // add appkey vào vendor model
            err = esp_ble_mesh_provisioner_bind_app_key_to_local_model(PROV_OWN_ADDR, prov_key.app_idx,
                                                                       ESP_BLE_MESH_VND_MODEL_ID_CLIENT, CID_ESP);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG_BLE, "Failed to bind AppKey to vendor client");
            }
        }
        break;
    case ESP_BLE_MESH_PROVISIONER_BIND_APP_KEY_TO_MODEL_COMP_EVT:
        ESP_LOGI(TAG_BLE, "ESP_BLE_MESH_PROVISIONER_BIND_APP_KEY_TO_MODEL_COMP_EVT, err_code %d", param->provisioner_bind_app_key_to_model_comp.err_code);
        break;
    case ESP_BLE_MESH_PROVISIONER_STORE_NODE_COMP_DATA_COMP_EVT:
        ESP_LOGI(TAG_BLE, "ESP_BLE_MESH_PROVISIONER_STORE_NODE_COMP_DATA_COMP_EVT, err_code %d", param->provisioner_store_node_comp_data_comp.err_code);
        break;
    case ESP_BLE_MESH_PROVISIONER_RECV_HEARTBEAT_MESSAGE_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_RECV_HEARTBEAT_MESSAGE_EVT, hbF_src 0x%04x, 0x%04x, %d, %d, %d, 0x%04x, %d", param->provisioner_recv_heartbeat.hb_src,
                 param->provisioner_recv_heartbeat.hb_dst, param->provisioner_recv_heartbeat.init_ttl, param->provisioner_recv_heartbeat.rx_ttl,
                 param->provisioner_recv_heartbeat.hops, param->provisioner_recv_heartbeat.feature, param->provisioner_recv_heartbeat.rssi);
        ble_mesh_recv_hb(param->provisioner_recv_heartbeat.hb_src);
    case ESP_BLE_MESH_MODEL_SUBSCRIBE_GROUP_ADDR_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_SUBSCRIBE_GROUP_ADDR_COMP_EVT, err_code %d, element_addr 0x%04x, company_id 0x%04x, model_id 0x%04x, group_addr 0x%04x",
                 param->model_sub_group_addr_comp.err_code, param->model_sub_group_addr_comp.element_addr,
                 param->model_sub_group_addr_comp.company_id, param->model_sub_group_addr_comp.model_id,
                 param->model_sub_group_addr_comp.group_addr);
        break;
    default:
        break;
    }
}

static void example_ble_mesh_parse_node_comp_data(const uint8_t *data, uint16_t length)
{
    uint16_t cid, pid, vid, crpl, feat;
    uint16_t loc, model_id, company_id;
    uint8_t nums, numv;
    uint16_t offset;
    int i;

    cid = COMP_DATA_2_OCTET(data, 0);
    pid = COMP_DATA_2_OCTET(data, 2);
    vid = COMP_DATA_2_OCTET(data, 4);
    crpl = COMP_DATA_2_OCTET(data, 6);
    feat = COMP_DATA_2_OCTET(data, 8);
    offset = 10;

    ESP_LOGI(TAG_BLE, "********************** Composition Data Start **********************");
    ESP_LOGI(TAG_BLE, "* CID 0x%04x, PID 0x%04x, VID 0x%04x, CRPL 0x%04x, Features 0x%04x *", cid, pid, vid, crpl, feat);
    for (; offset < length;)
    {
        loc = COMP_DATA_2_OCTET(data, offset);
        nums = COMP_DATA_1_OCTET(data, offset + 2);
        numv = COMP_DATA_1_OCTET(data, offset + 3);
        offset += 4;
        ESP_LOGI(TAG_BLE, "* Loc 0x%04x, NumS 0x%02x, NumV 0x%02x *", loc, nums, numv);
        for (i = 0; i < nums; i++)
        {
            model_id = COMP_DATA_2_OCTET(data, offset);
            ESP_LOGI(TAG_BLE, "* SIG Model ID 0x%04x *", model_id);
            offset += 2;
        }
        for (i = 0; i < numv; i++)
        {
            company_id = COMP_DATA_2_OCTET(data, offset);
            model_id = COMP_DATA_2_OCTET(data, offset + 2);
            ESP_LOGI(TAG_BLE, "* Vendor Model ID 0x%04x, Company ID 0x%04x *", model_id, company_id);
            offset += 4;
        }
    }
    ESP_LOGI(TAG_BLE, "*********************** Composition Data End ***********************");
}

uint8_t esp_ble_mesh_delete_node(uint16_t unicast)
{
    int err;
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_cfg_client_set_state_t set = {0};
    esp_ble_mesh_node_t *node = NULL;
    node = esp_ble_mesh_provisioner_get_node_with_addr(unicast);
    if (!node)
    {
        ESP_LOGE(TAG_BLE, "Failed to get node 0x%04x info", unicast);
        return;
    }
    example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_NODE_RESET);
    set.net_key_delete.net_idx = node->net_idx;
    ESP_LOGI(TAG_BLE, "Reseting node...");
    err = esp_ble_mesh_config_client_set_state(&common, &set);
    if (err)
    {
        ESP_LOGE(TAG_BLE, "%s: failed", __func__);
        return;
    }
}

// int cfg_client_add_group_address(uint16_t element_addr, uint16_t sub_addr, uint16_t model_id)
// {
//     int err;
//     esp_ble_mesh_client_common_param_t common = {0};
//     esp_ble_mesh_cfg_client_set_state_t set_state = {0};
//     esp_ble_mesh_set_msg_common(&common, element_addr, 0, 0, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD);
//     set_state.model_sub_add.element_addr = element_addr;
//     set_state.model_sub_add.sub_addr = sub_addr;
//     set_state.model_sub_add.model_id = model_id;
//     set_state.model_sub_add.company_id = CID_NVAL;
//     err = esp_ble_mesh_config_client_set_state(&common, &set_state);
//     if (err)
//     {
//         ESP_LOGE(TAGCFG, "%s: Config Model add subscription to group address to model", __func__);
//         return err;
//     }
//     return err;
// }
// static void ble_mesh_set_pub_msg_common(esp_ble_mesh_client_common_param_t *common,
//                                         esp_ble_mesh_node_t *node,
//                                         esp_ble_mesh_model_t *model, uint32_t opcode)
// {
//     example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_PUB_SET);
//     set_state.model_pub_set.element_addr = node->unicast_addr; // node->unicast;
//     set_state.model_pub_set.publish_addr = SUBSCRIPTION_ADDR;  // node->unicast+1;
//     set_state.model_pub_set.publish_app_idx = prov_key.app_idx;
//     set_state.model_pub_set.cred_flag = true;
//     set_state.model_pub_set.publish_ttl = 7;
//     set_state.model_pub_set.publish_period = 1;
//     set_state.model_pub_set.publish_retransmit = 1;
//     set_state.model_pub_set.model_id = ESP_BLE_MESH_VND_MODEL_ID_SERVER;
//     set_state.model_pub_set.company_id = CID_ESP;
//     err = esp_ble_mesh_config_client_set_state(&common, &set_state);
//     if (err)
//     {
//         ESP_LOGE(TAG, "%s: config client Set failed", __func__);
//         return;
//     }
// }

void ble_mesh_send_vendor_publish_message(void)
{
    esp_ble_mesh_model_t *model = NULL;
    uint32_t opcode = ESP_BLE_MESH_VND_MODEL_OP_SEND;
    // uint8_t data[3] = {};
    char data[10] = "anhbungdz";

    ESP_LOGI("BLE MESH HANDLE", "Inside gen_onoff_set_handler()");

    // model = esp_ble_mesh_find_vendor_model(&elements[0], CID_ESP, ESP_BLE_MESH_VND_MODEL_ID_CLIENT);
    // if (!model)
    // {
    //     ESP_LOGI("BLE MESH HANDLE", "K tim thay vendor Model");
    //     return ESP_ERR_INVALID_ARG;
    // }

    // Add the Group address into the model publish_addr in which the message
    // will be published
    // model->pub->publish_addr = 0xFFFF;

    vendor_client.model->pub->publish_addr = 0xFFFF;
    esp_ble_mesh_model_publish(vendor_client.model, opcode, sizeof(data), (uint8_t *)data, ROLE_NODE);
}

/**
 *  @brief Hàm này gọi lại khi có event tin nhắn từ node gửi đến
 *
 *  @param[in] event Event của callBack
 *  @param[in] param Thông tin của Node gửi đến
 *  @return none
 */
static void example_ble_mesh_config_client_cb(esp_ble_mesh_cfg_client_cb_event_t event,
                                              esp_ble_mesh_cfg_client_cb_param_t *param)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_cfg_client_set_state_t set = {0};
    esp_ble_mesh_node_t *node = NULL;
    esp_err_t err;

    ESP_LOGI(TAG, "Config client, err_code %d, event %u, addr 0x%04x, opcode 0x%04x",
             param->error_code, event, param->params->ctx.addr, param->params->opcode);

    if (param->error_code)
    {
        ESP_LOGE(TAG, "Send config client message failed, opcode 0x%04x", param->params->opcode);
        return;
    }

    node = esp_ble_mesh_provisioner_get_node_with_addr(param->params->ctx.addr);
    if (!node)
    {
        ESP_LOGE(TAG, "Failed to get node 0x%04x info", param->params->ctx.addr);
        return;
    }

    switch (event)
    {
    case ESP_BLE_MESH_CFG_CLIENT_GET_STATE_EVT:
        if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET)
        {
            ESP_LOG_BUFFER_HEX("Composition data", param->status_cb.comp_data_status.composition_data->data,
                               param->status_cb.comp_data_status.composition_data->len);
            example_ble_mesh_parse_node_comp_data(param->status_cb.comp_data_status.composition_data->data,
                                                  param->status_cb.comp_data_status.composition_data->len);
            err = esp_ble_mesh_provisioner_store_node_comp_data(param->params->ctx.addr,
                                                                param->status_cb.comp_data_status.composition_data->data,
                                                                param->status_cb.comp_data_status.composition_data->len);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to store node composition data");
                break;
            }

            example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
            set.app_key_add.net_idx = prov_key.net_idx;
            set.app_key_add.app_idx = prov_key.app_idx;
            memcpy(set.app_key_add.app_key, prov_key.app_key, ESP_BLE_MESH_OCTET16_LEN);
            err = esp_ble_mesh_config_client_set_state(&common, &set);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to send Config AppKey Add");
            }
        }
        break;
    case ESP_BLE_MESH_CFG_CLIENT_SET_STATE_EVT:
        if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD)
        {
            if (temp_pro == 1)
            {
                ESP_LOGI(TAG, "ESP_BLE_MESH_CFG_CLIENT_SET_STATE_EVT 2");
                example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
                set.model_app_bind.element_addr = node->unicast_addr;
                set.model_app_bind.model_app_idx = prov_key.app_idx;
                set.model_app_bind.model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SRV;
                set.model_app_bind.company_id = ESP_BLE_MESH_CID_NVAL;
                err = esp_ble_mesh_config_client_set_state(&common, &set);
                if (err != ESP_OK)
                {
                    ESP_LOGE(TAG, "Failed to send Config Model App Bind");
                    return;
                }
            }
            if (temp_pro == 2)
            {
                example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
                set.model_app_bind.element_addr = node->unicast_addr;
                set.model_app_bind.model_app_idx = prov_key.app_idx;
                set.model_app_bind.model_id = ESP_BLE_MESH_VND_MODEL_ID_SERVER;
                set.model_app_bind.company_id = CID_ESP;
                err = esp_ble_mesh_config_client_set_state(&common, &set);
                if (err != ESP_OK)
                {
                    ESP_LOGE(TAG, "Failed to send Config Model App Bind");
                }
                ESP_LOGI(TAG, " temp_pro = 2");
            }
        }
        else if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND)
        {
            if (param->status_cb.model_app_status.model_id == ESP_BLE_MESH_MODEL_ID_SENSOR_SRV &&
                param->status_cb.model_app_status.company_id == ESP_BLE_MESH_CID_NVAL)
            {
                ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_ID_SENSOR_SRV ");
                example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
                set.model_app_bind.element_addr = node->unicast_addr;
                set.model_app_bind.model_app_idx = prov_key.app_idx;
                set.model_app_bind.model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SETUP_SRV;
                set.model_app_bind.company_id = ESP_BLE_MESH_CID_NVAL;
                err = esp_ble_mesh_config_client_set_state(&common, &set);
                if (err)
                {
                    ESP_LOGE(TAG, "Failed to send Config Model App Bind");
                    return;
                }
            }
            else if (param->status_cb.model_app_status.model_id == ESP_BLE_MESH_MODEL_ID_SENSOR_SETUP_SRV &&
                     param->status_cb.model_app_status.company_id == ESP_BLE_MESH_CID_NVAL)
            {
                example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
                set.app_key_add.net_idx = prov_key.net_idx;
                set.app_key_add.app_idx = prov_key.app_idx;
                memcpy(set.app_key_add.app_key, prov_key.app_key, ESP_BLE_MESH_OCTET16_LEN);
                err = esp_ble_mesh_config_client_set_state(&common, &set);
                if (err != ESP_OK)
                {
                    ESP_LOGE(TAG, "Failed to send Config AppKey Add vendor");
                }
                temp_pro = 2;
            }
            else
            {
                example_ble_mesh_send_sensor_message(ESP_BLE_MESH_MODEL_OP_SENSOR_GET);
                ESP_LOGW(TAG, "Provision and config successfully");
            }
        }
        break;

    // case ESP_BLE_MESH_MODEL_OP_HEARTBEAT_PUB_SET:
    // {
    //     esp_ble_mesh_cfg_client_set_state_t set_state = {0};
    //     example_ble_mesh_set_msg_common(&common, node->unicast_addr, config_client.model, ESP_BLE_MESH_MODEL_OP_HEARTBEAT_PUB_SET);
    //     set_state.heartbeat_pub_set.dst = SUBSCRIBE_DEST;
    //     set_state.heartbeat_pub_set.count = 0xFF;   // Heartbeat messages are being sent indefinitely
    //     set_state.heartbeat_pub_set.period = 0x03;  // Heartbeat messages have a publication period of 4 seconds
    //     set_state.heartbeat_pub_set.ttl = 0x7F;     // Maximum allowed TTL value
    //     set_state.heartbeat_pub_set.feature = 0x03; // feature
    //     set_state.heartbeat_pub_set.net_idx = prov_key.net_idx;
    //     err = esp_ble_mesh_config_client_set_state(&common, &set_state);
    //     if (err)
    //     {
    //         ESP_LOGE(TAG, "%s: Config Heartbeat Publication Set failed", __func__);
    //         return;
    //     }
    //     break;
    // }
    case ESP_BLE_MESH_CFG_CLIENT_PUBLISH_EVT:
        if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_STATUS)
        {
            ESP_LOG_BUFFER_HEX("Composition data", param->status_cb.comp_data_status.composition_data->data,
                               param->status_cb.comp_data_status.composition_data->len);
        }
        break;
    case ESP_BLE_MESH_CFG_CLIENT_TIMEOUT_EVT:
        switch (param->params->opcode)
        {
        case ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET:
        {
            esp_ble_mesh_cfg_client_get_state_t get = {0};
            example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET);
            get.comp_data_get.page = COMP_DATA_PAGE_0;
            err = esp_ble_mesh_config_client_get_state(&common, &get);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to send Config Composition Data Get");
            }
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
            example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
            set.app_key_add.net_idx = prov_key.net_idx;
            set.app_key_add.app_idx = prov_key.app_idx;
            memcpy(set.app_key_add.app_key, prov_key.app_key, ESP_BLE_MESH_OCTET16_LEN);
            err = esp_ble_mesh_config_client_set_state(&common, &set);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to send Config AppKey Add");
            }
            break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
            example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
            set.model_app_bind.element_addr = node->unicast_addr;
            set.model_app_bind.model_app_idx = prov_key.app_idx;
            set.model_app_bind.model_id = ESP_BLE_MESH_VND_MODEL_ID_SERVER;
            set.model_app_bind.company_id = CID_ESP;
            err = esp_ble_mesh_config_client_set_state(&common, &set);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to send Config Model App Bind");
            }

            break;
        default:
            break;
        }
        break;
    default:
        ESP_LOGE(TAG, "Invalid config client event %u", event);
        break;
    }
}

// /**
//  *  @brief Hàm này gọi lại khi có event tin nhắn từ node gửi đến
//  *
//  *  @param[in] event Event của callBack
//  *  @param[in] param Thông tin của Node gửi đến
//  *  @return none
//  */
// static void example_ble_mesh_config_client_cb(esp_ble_mesh_cfg_client_cb_event_t event,
//                                               esp_ble_mesh_cfg_client_cb_param_t *param)
// {
//     esp_ble_mesh_client_common_param_t common = {0};
//     esp_ble_mesh_cfg_client_set_state_t set = {0};
//     esp_ble_mesh_node_t *node = NULL;
//     esp_err_t err;

//     ESP_LOGI(TAG_BLE, "Config client, err_code %d, event %u, addr 0x%04x, opcode 0x%04x",
//              param->error_code, event, param->params->ctx.addr, param->params->opcode);

//     if (param->error_code)
//     {
//         ESP_LOGE(TAG_BLE, "Send config client message failed, opcode 0x%04x", param->params->opcode);
//         return;
//     }

//     node = esp_ble_mesh_provisioner_get_node_with_addr(param->params->ctx.addr);
//     if (!node)
//     {
//         ESP_LOGE(TAG_BLE, "Failed to get node 0x%04x info", param->params->ctx.addr);
//         return;
//     }

//     switch (event)
//     {
//     case ESP_BLE_MESH_CFG_CLIENT_GET_STATE_EVT:
//         if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET)
//         {
//             ESP_LOG_BUFFER_HEX("Composition data", param->status_cb.comp_data_status.composition_data->data,
//                                param->status_cb.comp_data_status.composition_data->len);
//             example_ble_mesh_parse_node_comp_data(param->status_cb.comp_data_status.composition_data->data,
//                                                   param->status_cb.comp_data_status.composition_data->len);
//             err = esp_ble_mesh_provisioner_store_node_comp_data(param->params->ctx.addr,
//                                                                 param->status_cb.comp_data_status.composition_data->data,
//                                                                 param->status_cb.comp_data_status.composition_data->len);
//             if (err != ESP_OK)
//             {
//                 ESP_LOGE(TAG_BLE, "Failed to store node composition data");
//                 break;
//             }

//             example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
//             set.app_key_add.net_idx = prov_key.net_idx;
//             set.app_key_add.app_idx = prov_key.app_idx;
//             // set.heartbeat_pub_set.dst = SUBSCRIBE_DEST;
//             // set.heartbeat_pub_set.count = 0xFF;   // Heartbeat messages are being sent indefinitely
//             // set.heartbeat_pub_set.period = 0x03;  // Heartbeat messages have a publication period of 4 seconds
//             // set.heartbeat_pub_set.ttl = 0x7F;     // Maximum allowed TTL value
//             // set.heartbeat_pub_set.feature = 0x03; // feature
//             // set.heartbeat_pub_set.net_idx = prov_key.net_idx;
//             memcpy(set.app_key_add.app_key, prov_key.app_key, ESP_BLE_MESH_OCTET16_LEN);
//             err = esp_ble_mesh_config_client_set_state(&common, &set);
//             if (err != ESP_OK)
//             {
//                 ESP_LOGE(TAG_BLE, "Failed to send Config AppKey Add");
//             }
//         }
//         break;
//     case ESP_BLE_MESH_CFG_CLIENT_SET_STATE_EVT:
//         if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_HEARTBEAT_PUB_SET)
//         {
//             ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_HEARTBEAT_PUB_SET 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",
//                      param->status_cb.heartbeat_pub_status.status, param->status_cb.heartbeat_pub_status.dst, param->status_cb.heartbeat_pub_status.count,
//                      param->status_cb.heartbeat_pub_status.period, param->status_cb.heartbeat_pub_status.ttl, param->status_cb.heartbeat_pub_status.features,
//                      param->status_cb.heartbeat_pub_status.net_idx);

//             esp_ble_mesh_cfg_client_set_state_t set_state = {0};
//             example_ble_mesh_set_msg_common(&common, node->unicast_addr, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
//             set_state.app_key_add.net_idx = prov_key.net_idx;
//             set_state.app_key_add.app_idx = prov_key.app_idx;
//             memcpy(set_state.app_key_add.app_key, prov_key.app_key, 16);
//             err = esp_ble_mesh_config_client_set_state(&common, &set_state);
//             if (err)
//             {
//                 ESP_LOGE(TAG, "%s: Config AppKey Add failed", __func__);
//                 return;
//             }
//         }
//         if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD)
//         {
//             if (temp_pro == 1)
//             {
//                 ESP_LOGI(TAG_BLE, "Da App key local to Sensor Server\na ");
//                 example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
//                 set.model_app_bind.element_addr = node->unicast_addr;
//                 set.model_app_bind.model_app_idx = prov_key.app_idx;
//                 set.model_app_bind.model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SRV;
//                 set.model_app_bind.company_id = ESP_BLE_MESH_CID_NVAL;
//                 err = esp_ble_mesh_config_client_set_state(&common, &set);
//                 if (err != ESP_OK)
//                 {
//                     ESP_LOGE(TAG_BLE, "Failed to send Config Model App Bind");
//                     return;
//                 }
//             }
//             if (temp_pro == 2)
//             {
//                 ESP_LOGI(TAG_BLE, "Da App key local to Vendor Server\n");
//                 example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
//                 set.model_app_bind.element_addr = node->unicast_addr;
//                 set.model_app_bind.model_app_idx = prov_key.app_idx;
//                 set.model_app_bind.model_id = ESP_BLE_MESH_VND_MODEL_ID_SERVER;
//                 set.model_app_bind.company_id = CID_ESP;
//                 err = esp_ble_mesh_config_client_set_state(&common, &set);
//                 if (err != ESP_OK)
//                 {
//                     ESP_LOGE(TAG_BLE, "Failed to send Config Model App Bind");
//                 }
//             }
//         }
//         else if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND)
//         {
//             if (param->status_cb.model_app_status.model_id == ESP_BLE_MESH_MODEL_ID_SENSOR_SRV &&
//                 param->status_cb.model_app_status.company_id == ESP_BLE_MESH_CID_NVAL)
//             {
//                 ESP_LOGI(TAG_BLE, "Da App key local to Sensor Setup Server \n");
//                 example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
//                 set.model_app_bind.element_addr = node->unicast_addr;
//                 set.model_app_bind.model_app_idx = prov_key.app_idx;
//                 // set.model_pub_set.element_addr = node->unicast_addr;
//                 // set.model_pub_set.publish_addr = SUBSCRIBE_DEST;
//                 // set.model_pub_set.publish_app_idx = prov_key.app_idx;
//                 // set.model_pub_set.cred_flag = 0;
//                 // set.model_pub_set.publish_ttl = 0x7F;                    // Maximum allowed TTL value
//                 // set.model_pub_set.publish_period = 0x02 << 6 | 0x01;     /* Number of Steps & Step Resolution*/
//                 // set.model_pub_set.publish_retransmit = 0x10 << 3 | 0x01; /* Retransmit Count & Interval Steps. */
//                 // set.model_pub_set.model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SRV;
//                 // set.model_pub_set.company_id = ESP_BLE_MESH_CID_NVAL;
//                 set.model_app_bind.model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SETUP_SRV;
//                 set.model_app_bind.company_id = ESP_BLE_MESH_CID_NVAL;
//                 err = esp_ble_mesh_config_client_set_state(&common, &set);
//                 if (err)
//                 {
//                     ESP_LOGE(TAG_BLE, "Failed to send Config Model App Bind");
//                     return;
//                 }
//             }
//             else if (param->status_cb.model_app_status.model_id == ESP_BLE_MESH_MODEL_ID_SENSOR_SETUP_SRV &&
//                      param->status_cb.model_app_status.company_id == ESP_BLE_MESH_CID_NVAL)
//             {
//                 example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
//                 set.app_key_add.net_idx = prov_key.net_idx;
//                 set.app_key_add.app_idx = prov_key.app_idx;
//                 memcpy(set.app_key_add.app_key, prov_key.app_key, ESP_BLE_MESH_OCTET16_LEN);
//                 err = esp_ble_mesh_config_client_set_state(&common, &set);
//                 if (err != ESP_OK)
//                 {
//                     ESP_LOGE(TAG_BLE, "Failed to send Config AppKey Add vendor");
//                 }
//                 temp_pro = 2;
//             }
//             else
//             {
//                 // example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_PUB_SET);
//                 // set.model_pub_set.element_addr = node->unicast_addr;
//                 // set.model_pub_set.publish_addr = SUBSCRIPTION_ADDR;
//                 // set.model_pub_set.publish_app_idx = prov_key.app_idx;
//                 // set.model_pub_set.publish_ttl = 7;
//                 // set.model_pub_set.model_id = ESP_BLE_MESH_VND_MODEL_ID_CLIENT;
//                 // set.model_pub_set.company_id = CID_ESP;
//                 // err = esp_ble_mesh_config_client_set_state(&common, &set);
//                 example_ble_mesh_send_sensor_message(ESP_BLE_MESH_MODEL_OP_SENSOR_GET);
//                 //uint8_t match[2] = {0xFF, 0x10}; // 2 byte đầu của UUID
//                 //esp_ble_mesh_provisioner_set_dev_uuid_match(match, sizeof(match), 0x0, false);
//                 ESP_LOGW(TAG_BLE, "Provision and config successfully");
//                 // esp_wifi_start();
//                 // EventBits_t uxBits = xEventGroupWaitBits(s_wifi_event_group,
//                 //                                          WIFI_CONNECTED_BIT,
//                 //                                          pdTRUE,
//                 //                                          pdFALSE,
//                 //                                          portMAX_DELAY);
//                 // if ((uxBits & WIFI_CONNECTED_BIT) == WIFI_CONNECTED_BIT)
//                 // {
//                 // TaskHandle_t xTaskRtcHandle;
//                 // StaticTask_t xTaskWsHandle;
//                 // StaticTask_t xTaskNetworkHandle;
//                 // vTaskResume(xTaskWsHandle);
//                 // vTaskResume(xTaskNetworkHandle);
//                 //}
//                 // esp_ble_mesh_provisioner_prov_disable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
//             }
//         }
//         if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_MODEL_PUB_SET)
//         {
//             ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_PUB_SET");
//             // esp_ble_mesh_generic_client_get_state_t get_state = {0};
//             // ble_mesh_set_msg_common(&common, node->unicast, onoff_client.model, ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET);
//             // err = esp_ble_mesh_generic_client_get_state(&common, &get_state);
//             // if (err)
//             // {
//             //     ESP_LOGE(TAG, "%s: Generic OnOff Get failed", __func__);
//             //     return;
//             // }
//         }
//         break;
//     case ESP_BLE_MESH_CFG_CLIENT_PUBLISH_EVT:
//         if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_STATUS)
//         {
//             ESP_LOG_BUFFER_HEX("Composition data", param->status_cb.comp_data_status.composition_data->data,
//                                param->status_cb.comp_data_status.composition_data->len);
//         }
//         break;
//     case ESP_BLE_MESH_CFG_CLIENT_TIMEOUT_EVT:
//         switch (param->params->opcode)
//         {
//         case ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET:
//         {
//             esp_ble_mesh_cfg_client_get_state_t get = {0};
//             example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET);
//             get.comp_data_get.page = COMP_DATA_PAGE_0;
//             err = esp_ble_mesh_config_client_get_state(&common, &get);
//             if (err != ESP_OK)
//             {
//                 ESP_LOGE(TAG_BLE, "Failed to send Config Composition Data Get");
//             }
//             break;
//         }
//         case ESP_BLE_MESH_MODEL_OP_HEARTBEAT_PUB_SET:
//         {
//             esp_ble_mesh_cfg_client_set_state_t set_state = {0};
//             example_ble_mesh_set_msg_common(&common, node->unicast_addr, config_client.model, ESP_BLE_MESH_MODEL_OP_HEARTBEAT_PUB_SET);
//             set_state.heartbeat_pub_set.dst = SUBSCRIBE_DEST;
//             set_state.heartbeat_pub_set.count = 0xFF;   // Heartbeat messages are being sent indefinitely
//             set_state.heartbeat_pub_set.period = 0x03;  // Heartbeat messages have a publication period of 4 seconds
//             set_state.heartbeat_pub_set.ttl = 0x7F;     // Maximum allowed TTL value
//             set_state.heartbeat_pub_set.feature = 0x03; // feature
//             set_state.heartbeat_pub_set.net_idx = prov_key.net_idx;
//             err = esp_ble_mesh_config_client_set_state(&common, &set_state);
//             if (err)
//             {
//                 ESP_LOGE(TAG, "%s: Config Heartbeat Publication Set failed", __func__);
//                 return;
//             }
//             break;
//         }
//         case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
//             example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
//             set.app_key_add.net_idx = prov_key.net_idx;
//             set.app_key_add.app_idx = prov_key.app_idx;
//             memcpy(set.app_key_add.app_key, prov_key.app_key, ESP_BLE_MESH_OCTET16_LEN);
//             err = esp_ble_mesh_config_client_set_state(&common, &set);
//             if (err != ESP_OK)
//             {
//                 ESP_LOGE(TAG_BLE, "Failed to send Config AppKey Add");
//             }
//             break;
//         case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
//             example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
//             set.model_app_bind.element_addr = node->unicast_addr;
//             set.model_app_bind.model_app_idx = prov_key.app_idx;
//             set.model_app_bind.model_id = ESP_BLE_MESH_VND_MODEL_ID_SERVER;
//             set.model_app_bind.company_id = CID_ESP;
//             err = esp_ble_mesh_config_client_set_state(&common, &set);
//             if (err != ESP_OK)
//             {
//                 ESP_LOGE(TAG_BLE, "Failed to send Config Model App Bind");
//             }

//             break;
//         case ESP_BLE_MESH_MODEL_OP_MODEL_PUB_SET:
//         {
//             esp_ble_mesh_cfg_client_set_state_t set_state = {0};
//             example_ble_mesh_set_msg_common(&common, node->unicast_addr, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_PUB_SET);
//             set_state.app_key_add.net_idx = prov_key.net_idx;
//             set_state.app_key_add.app_idx = prov_key.app_idx;
//             // set_state.model_pub_set.element_addr = node->unicast_addr;
//             // set_state.model_pub_set.publish_addr = SUBSCRIBE_DEST;
//             // set_state.model_pub_set.publish_app_idx = prov_key.app_idx;
//             // set_state.model_pub_set.cred_flag = 0;
//             // set_state.model_pub_set.publish_ttl = 0x7F;                    // Maximum allowed TTL value
//             // set_state.model_pub_set.publish_period = 0x02 << 6 | 0x01;     /* Number of Steps & Step Resolution*/
//             // set_state.model_pub_set.publish_retransmit = 0x10 << 3 | 0x01; /* Retransmit Count & Interval Steps. */
//             // set_state.model_pub_set.model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SETUP_SRV;
//             // set_state.model_pub_set.company_id = ESP_BLE_MESH_CID_NVAL;
//             err = esp_ble_mesh_config_client_set_state(&common, &set_state);
//             if (err)
//             {
//                 ESP_LOGE(TAG, "%s: Config Model Publication Set failed", __func__);
//                 return;
//             }
//             break;
//         }
//         default:
//             break;
//         }
//         break;
//     default:
//         ESP_LOGE(TAG_BLE, "Invalid config client event %u", event);
//         break;
//     }
// }

/**
 *  @brief Hàm gửi tin nhắn đến node cụ thể theo unicast address
 *
 */
void example_ble_mesh_send_vendor_message(uint16_t addr, void *data)
{
    esp_ble_mesh_msg_ctx_t ctx = {0};
    uint32_t opcode;
    esp_err_t err;

    ctx.net_idx = prov_key.net_idx;
    ctx.app_idx = prov_key.app_idx;
    ctx.addr = addr;
    ctx.send_ttl = MSG_SEND_TTL;
    ctx.send_rel = MSG_SEND_REL;
    // ESP_BLE_MESH_VND_MODEL_OP_SEND
    opcode = ESP_BLE_MESH_VND_MODEL_OP_SEND;
    // opcode = 0; // opcode để phân loại tin nhắn
    ESP_LOGI(TAG_BLE, "Da gui tn den node %02x", addr);
    err = esp_ble_mesh_client_model_send_msg(vendor_client.model, &ctx, (uint32_t)opcode,
                                             strlen(data), (uint8_t *)data, MSG_TIMEOUT, false, MSG_ROLE); // gửi tin nhắn kèm opcode
    ESP_LOGI(TAG_BLE, "Farme: %.*s\n", strlen(data), (char *)data);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_BLE, "Failed to send vendor message 0x%06x", opcode);
        return;
    }
}
/**
 *  @brief Khi quá time mà không gửi đc tin nhắn
 *
 *  @param[in] opcode opcode
 *  @return none
 */
static void example_ble_mesh_sensor_timeout(uint32_t opcode)
{
    example_ble_mesh_send_sensor_message(opcode);
}

/**
 *  @brief Hàm này gọi lại khi có event tin nhắn từ node gửi đến
 *
 *  @param[in] event Event của callBack
 *  @param[in] param Thông tin của Node gửi đến
 *  @return none
 */
static void example_ble_mesh_sensor_client_cb(esp_ble_mesh_sensor_client_cb_event_t event,
                                              esp_ble_mesh_sensor_client_cb_param_t *param)
{
    esp_ble_mesh_node_t *node = NULL;
    char data[30] = {0};
    uint8_t match[2] = {0xFF, 0x10}; // 2 byte đầu của UUID
    // printf("RSSI: %d \n", param->params->ctx.recv_rssi);
    ESP_LOGI(TAG_BLE, "Sensor client, event %u, addr 0x%04x", event, param->params->ctx.addr);
    rx_node = param->params->ctx.addr - 4;
    if (param->error_code)
    {
        ESP_LOGE(TAG_BLE, "Send sensor client message failed (err %d)", param->error_code);
        return;
    }

    node = esp_ble_mesh_provisioner_get_node_with_addr(param->params->ctx.addr);
    if (!node)
    {
        ESP_LOGE(TAG_BLE, "Node 0x%04x not exists", param->params->ctx.addr);
        return;
    }

    switch (event)
    {
    case ESP_BLE_MESH_SENSOR_CLIENT_GET_STATE_EVT:
        switch (param->params->opcode)
        {
        case ESP_BLE_MESH_MODEL_OP_SENSOR_GET:
            ESP_LOGI(TAG_BLE, "Sensor Status, opcode 0x%04x", param->params->ctx.recv_op);
            if (param->status_cb.sensor_status.marshalled_sensor_data->len)
            {
                ESP_LOGI(TAG, "Sensor Data: %.*s\n", param->status_cb.sensor_status.marshalled_sensor_data->len, (char *)(param->status_cb.sensor_status.marshalled_sensor_data->data));
                //  vTaskResume(xTaskRtcHandle);
                //   vTaskResume(xTaskWsHandle);
                //    vTaskResume(xTaskNetworkHandle);
                esp_mqtt_client_publish(clientMqtt[0], "v1/devices/me/attributes", data_mesh_info, 0, 1, 0); // shared attribute
                insertMeshNode(&meshDataPrevious, role, unicast, "No", mac);
                websocket_lister_resigter(server_addr - 4);
                isFirstRxWs[server_addr - 4] = 1;
                mqtt_connect(server_addr - 4);
                // memcpy(data_mesh_rx, (uint8_t *)(param->status_cb.sensor_status.marshalled_sensor_data->data), param->status_cb.sensor_status.marshalled_sensor_data->len);
                // printf("Data: %s\n", data_mesh_rx + 2);
                example_ble_mesh_send_vendor_message(param->params->ctx.addr, data_ack); // gửi lại ack
                // printf("Temperature: %3d.%1d C\n", INT_TEMP(data_mesh_rx[2]), FRAC_TEMP(data_mesh_rx[2]));
                //  printf("%d-%s\n", param->status_cb.sensor_status.marshalled_sensor_data->len, data_temp2);

                esp_ble_mesh_provisioner_set_dev_uuid_match(match, sizeof(match), 0x0, false); // chỉ cấp phép khi match UUID do gateway quy định
                // esp_ble_mesh_provisioner_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
            }
            break;
        default:
            ESP_LOGE(TAG_BLE, "Unknown Sensor Get opcode 0x%04x", param->params->ctx.recv_op);
            break;
        }
        break;
    case ESP_BLE_MESH_SENSOR_CLIENT_SET_STATE_EVT:
        switch (param->params->opcode)
        {
        default:
            ESP_LOGE(TAG_BLE, "Unknown Sensor Set opcode 0x%04x", param->params->ctx.recv_op);
            break;
        }
        break;
    case ESP_BLE_MESH_SENSOR_CLIENT_PUBLISH_EVT:
        ESP_LOGI(TAG_BLE, "ESP_BLE_MESH_SENSOR_CLIENT_PUBLISH_EVT\n");
        // example_ble_mesh_send_vendor_message(param->params->ctx.addr, data_ack); // gửi lại ack
        //  memset(data_mesh_rx, 0, strlen(data_mesh_rx));
        sprintf(data_mesh_rx, "%.*s", param->status_cb.sensor_status.marshalled_sensor_data->len, (char *)(param->status_cb.sensor_status.marshalled_sensor_data->data));
        // memcpy(data_mesh_rx, (uint8_t *)(param->status_cb.sensor_status.marshalled_sensor_data->data), param->status_cb.sensor_status.marshalled_sensor_data->len);
        // printf("Data: %s ---Lenght:%d\n ", data_mesh_rx + 2, param->status_cb.sensor_status.marshalled_sensor_data->len - 2);
        printf("Data: %s ---Lenght:%d\n ", data_mesh_rx, param->status_cb.sensor_status.marshalled_sensor_data->len);
        if (strstr(data_mesh_rx, "ACK") != NULL)
        {
            sprintf(data, "{\"zlife\":1}");
            zlife_node[rx_node] = 1;
            esp_mqtt_client_publish(clientMqtt[rx_node], "v1/devices/me/telemetry", data, 0, 1, 0);
            isJustSendThingsBoard = 1;
        }
        else
        {
            xEventGroupSetBits(s_mesh_network_event_group, MESH_MESSAGE_ARRIVE_BIT);
        }
        // example_ble_mesh_send_vendor_message(param->params->ctx.addr, data_ack); // gửi lại ack

        break;
    case ESP_BLE_MESH_SENSOR_CLIENT_TIMEOUT_EVT:
        example_ble_mesh_sensor_timeout(param->params->opcode);
    default:
        break;
    }
}

/**
 *  @brief Gửi thông tin gateway về sensor model lần đầu tiên đến các node
 *
 */
void example_ble_mesh_send_sensor_message(uint32_t opcode)
{
    esp_ble_mesh_sensor_client_get_state_t get = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_node_t *node = NULL;
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG_BLE, "Da gui tin nhan den node 0x%04X\n", server_addr);
    node = esp_ble_mesh_provisioner_get_node_with_addr(server_addr); // gửi theo Unicast address
    if (node == NULL)
    {
        ESP_LOGE(TAG_BLE, "Node 0x%04x not exists", server_addr);
        return;
    }
    example_ble_mesh_set_msg_common(&common, node, sensor_client.model, opcode); // gửi các tham số liên quan đến sensor model

    err = esp_ble_mesh_sensor_client_get_state(&common, &get);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_BLE, "Failed to send sensor message 0x%04x", opcode);
    }
}

static void example_ble_mesh_custom_model_cb(esp_ble_mesh_model_cb_event_t event,
                                             esp_ble_mesh_model_cb_param_t *param)
{
    // static int64_t start_time;

    switch (event)
    {
    case ESP_BLE_MESH_MODEL_OPERATION_EVT:
        if (param->model_operation.opcode == ESP_BLE_MESH_VND_MODEL_OP_STATUS)
        {
            // int64_t end_time = esp_timer_get_time();
            // ESP_LOGI(TAG_BLE, "Sensor Node, event %u, addr 0x%04x", event, param->model_operation.ctx->addr);
            // ESP_LOGI(TAG_BLE, "Recv 0x%06x, tid 0x%04x, time %lldus",
            //          param->model_operation.opcode, store.vnd_tid, end_time - start_time);
            // ESP_LOGI(TAG_BLE, "Mess_Node: %.*s\n", param->model_operation.length, (char *)(param->model_operation.msg));
        }
        break;
    case ESP_BLE_MESH_MODEL_SEND_COMP_EVT:
        if (param->model_send_comp.err_code)
        {
            ESP_LOGE(TAG_BLE, "Failed to send message 0x%06x", param->model_send_comp.opcode);
            break;
        }
        // start_time = esp_timer_get_time();
        break;
    case ESP_BLE_MESH_MODEL_PUBLISH_COMP_EVT:
        ESP_LOGI(TAG_BLE, "ESP_BLE_MESH_MODEL_PUBLISH_COMP_EVT, err_code %d",
                 param->model_publish_comp.err_code);
        if (param->model_publish_comp.err_code)
        {
            ESP_LOGE(TAG_BLE, "Failed to publish message ");
            break;
        }
        rx_mess_check = 0;
        break;
    case ESP_BLE_MESH_CLIENT_MODEL_RECV_PUBLISH_MSG_EVT:
        // ESP_LOGI(TAG_BLE, "Receive publish message 0x%06x", param->client_recv_publish_msg.opcode);
        break;
    case ESP_BLE_MESH_CLIENT_MODEL_SEND_TIMEOUT_EVT:
        ESP_LOGW(TAG_BLE, "Client message 0x%06x timeout", param->client_send_timeout.opcode);
        break;
    default:
        break;
    }
}

/**
 *  @brief Khởi tạo BLE Mesh module và đăng ký các hàm callback xử lý event
 *
 */
esp_err_t ble_mesh_init(void)
{
    uint8_t match[2] = {0xFF, 0x10}; // 2 byte đầu của UUID
    esp_err_t err;
    prov_key.net_idx = ESP_BLE_MESH_KEY_PRIMARY;
    prov_key.app_idx = APP_KEY_IDX;
    memset(prov_key.app_key, APP_KEY_OCTET, sizeof(prov_key.app_key));

    esp_ble_mesh_register_prov_callback(example_ble_mesh_provisioning_cb);           // function callback xử lý event cấp phép của Gateway
    esp_ble_mesh_register_config_client_callback(example_ble_mesh_config_client_cb); // function callback xử lý các event liên kết Model
    esp_ble_mesh_register_custom_model_callback(example_ble_mesh_custom_model_cb);
    esp_ble_mesh_register_sensor_client_callback(example_ble_mesh_sensor_client_cb);                  // function callback xử lý tin nhắn từ Node
    esp_ble_mesh_init(&provision, &composition);                                                      // Initialize BLE Mesh module
    esp_ble_mesh_client_model_init(&vnd_models[0]);                                                   // Initialize Model Sensor và Vender
    esp_ble_mesh_provisioner_set_dev_uuid_match(match, sizeof(match), 0x0, false);                    // chỉ cấp phép khi match UUID do gateway quy định
    esp_ble_mesh_provisioner_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);             // Bắt đầu scanner các gói quảng cáo
    esp_ble_mesh_provisioner_add_local_app_key(prov_key.app_key, prov_key.net_idx, prov_key.app_idx); // add a local AppKey for Provisioner.
    err = esp_ble_mesh_provisioner_recv_heartbeat(true);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to enable recv heartbeat (err %d)", err);
        return err;
    }

    err = esp_ble_mesh_provisioner_set_heartbeat_filter_type(ESP_BLE_MESH_HEARTBEAT_FILTER_ACCEPTLIST);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to enable set heartbeat filter type (err %d)", err);
        return err;
    }

    esp_ble_mesh_heartbeat_filter_info_t heartbeat_filter_info = {
        .hb_src = BLE_MESH_ADDR_UNASSIGNED,
        .hb_dst = SUBSCRIBE_DEST,
    };
    err = esp_ble_mesh_provisioner_set_heartbeat_filter_info(ESP_BLE_MESH_HEARTBEAT_FILTER_ADD, &heartbeat_filter_info);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to enable set heartbeat filter info (err %d)", err);
        return err;
    }
    // gửi đến tất cả các Node thì gửi đến địa chỉ 0xFFFF
    // esp_ble_mesh_get_primary_element_address()
    // err = esp_ble_mesh_model_subscribe_group_addr(0X0001, CID_ESP, ESP_BLE_MESH_MODEL_ID_SENSOR_CLI, SUBSCRIBE_DEST);
    // if (err != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Failed to subscribe group addr (err %d)", err);
    //     return err;
    // }
    ESP_LOGI(TAG_BLE, "ESP BLE Mesh Provisioner initialized");
    return ESP_OK;
}