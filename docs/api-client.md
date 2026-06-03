# EFTP 客户端接口文档

**HOST**: `eftp.edianyun.com/api`（生产） / `test-eftp.edianyun.com/api`（测试）

**通用响应格式**:
```json
{
    "code": 0,       // 0=成功, 非0=失败
    "data": {},      // 业务数据
    "message": ""    // 提示信息
}
```

---

## 一、设备信息（3个）

### 1. 查询设备信息
- **接口**: `GET /client/queryDeviceInfo`
- **说明**: 启动时第一个调用，获取设备基础信息

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| sn | 否 | string | 设备序列号 |
| mac | 否 | string | MAC地址 |
| mainBoardSn | 否 | string | 主板序列号 |

**响应 data**:

| 字段 | 类型 | 说明 |
|------|------|------|
| sn | string | 序列号 |
| skuCode | string | SKU编码 |
| skuName | string | SKU名称 |
| spuName | string | 机型 |
| areaName | string | 工厂区域 |
| routeProcessesName | string | 当前工序 |
| workOrderNo | string | 工单号 |
| targetSku | string | 目标SKU |
| targetSkuName | string | 目标SKU名称 |
| isStandard | string | 是否标准品 |
| remark | string | 备注 |

---

### 2. 查询工序
- **接口**: `GET /client/queryDeviceRouteInfo`
- **说明**: 获取设备对应的工单及工艺路线

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| sn | 否 | string | 设备序列号 |
| mac | 否 | string | MAC地址 |

**响应 data**:

| 字段 | 类型 | 说明 |
|------|------|------|
| sn | string | 序列号 |
| routeId | int | 工艺路线ID |
| routeName | string | 工艺路线名称 |
| routeProcessesId | int | 工序ID（用于WiFi路由映射） |
| routeProcessesName | string | 工序名称 |
| orderType | int | 工单类型 |
| orderTypeName | string | 工单类型名称 |
| productType | int | 产品类别 |
| area | int | 工厂区域 |
| workOrderNo | string | 工单号 |
| workOrderType | int | 工单类型 |
| workOrderTypeName | string | 工单类型名称 |

---

### 3. 绑定SN和MAC
- **接口**: `POST /client/addMacAddr`
- **说明**: 设备SN与MAC地址绑定（服务端返回1001/1003时调用）

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| sn | 是 | string | 设备序列号 |
| mac | 是 | string | 有线网卡MAC |
| mainBoardSn | 是 | string | 主板序列号 |
| type | 是 | int | 类型（1或2） |

**响应**: 仅 code + message

---

## 二、测试计划（3个）

### 4. 查询测试计划（标准）
- **接口**: `GET /client/queryDeviceTestInfo`
- **说明**: 获取测试工艺、执行阶段、测试项信息

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| sn | 否 | string | 设备序列号 |
| mac | 否 | string | MAC地址 |
| mainBoardSn | 否 | string | 主板序列号 |

**响应 data**:

| 字段 | 类型 | 说明 |
|------|------|------|
| cycleId | long | 测试周期ID |
| technologyId | long | 测试工艺ID |
| isInit | int | 是否需要初始化：0=否, 1=是 |
| initStatus | int | 初始化状态：0=未开始, 1=进行中, 2=已完成 |
| isAutoExecute | int | 是否自动执行：0=否, 1=是 |
| status | int | 状态：0=未开始, 1=测试中, 2=已完成, 3=已关闭 |
| sn | string | 序列号 |
| testDeviceCycleStageVOList | array | 执行阶段列表 |
| testDeviceInitInfoVO | object | 初始化信息 |

**阶段对象 (testDeviceCycleStageVOList[])**:

| 字段 | 类型 | 说明 |
|------|------|------|
| id | int | 主键ID |
| stageId | int | 阶段ID |
| stageName | string | 阶段名称 |
| deviceCycleStageId | int | 生命周期阶段ID |
| deviceCycleId | int | 设备生命周期ID |
| isAutoExecute | int | 是否自动执行 |
| status | int | 状态：0=未开始, 1=进行中, 2=已结束 |
| result | string | 结果 |
| startTime | string | 开始时间 |
| endTime | string | 结束时间 |
| testDeviceCycleItemVOList | array | 测试项列表 |

**测试项对象 (testDeviceCycleItemVOList[])**:

| 字段 | 类型 | 说明 |
|------|------|------|
| id | int | 主键ID |
| itemId | int | 测试项ID |
| itemName | string | 测试项名称 |
| deviceCycleItemId | int | 生命周期测试项ID |
| deviceCycleStageId | int | 执行阶段生命周期ID |
| deviceCycleId | int | 测试生命周期ID |
| stageId | int | 执行阶段ID |
| technologyId | int | 测试工艺ID |
| callHref | string | 程序入口路径 |
| extractHref | string | 解压路径 |
| programType | int | 程序类型：1=exe, 2=bat |
| isAutoExecute | int | 是否自动执行 |
| isOffline | int | 是否离线：0=离线(U盘), 1=在线 |
| isRepeatTest | int | 支持重测：0=否, 1=是 |
| productType | int | 产品类型 |
| status | int | 状态：0=未开始, 1=进行中, 2=已完成 |
| result | int | 结果：0=无, 1=通过, 2=失败 |
| tips | string | 知识库提示 |
| testDeviceCycleItemResultVOList | array | 规则执行结果 |

**规则结果 (testDeviceCycleItemResultVOList[])**:

| 字段 | 类型 | 说明 |
|------|------|------|
| ruleName | string | 规则名称 |
| result | int | 结果：0=失败, 1=通过 |
| errorInfo | string | 错误信息 |
| testStandard | string | 检测标准 |
| testDeviceCycleItemDataVOList | array | 关联数据 |

**初始化信息 (testDeviceInitInfoVO)**:

| 字段 | 类型 | 说明 |
|------|------|------|
| dirWhiteList | string | 磁盘白名单 |
| testItemVOList | array | 初始化测试项列表 |

**初始化测试项 (testItemVOList[])**:

| 字段 | 类型 | 说明 |
|------|------|------|
| id | int | 主键ID |
| itemName | string | 测试项名称 |
| downLoadPath | string | 程序包下载路径 |
| unzipPath | string | 解压路径 |
| checkVersionResult | string | 版本是否匹配：0=不匹配, 1=匹配 |

---

### 5. 查询测试计划（仓库模式）
- **接口**: `GET /client/queryDeviceTestInfoForWareHouse`

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| sn | 否 | string | 设备序列号 |
| mac | 否 | string | MAC地址 |

---

### 6. 判断测试周期是否结束
- **接口**: `POST /client/judgeCurrentCycleEnd`
- **说明**: 轮询判断当前测试周期是否全部完成

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| sn | 否 | string | 设备序列号 |
| mac | 否 | string | MAC地址 |

**响应**: 仅 code + message + data

---

## 三、阶段生命周期（2个）

### 7. 初始化开始/结束
- **接口**: `POST /client/initStartOrEnd`
- **说明**: 初始化阶段的开始和结束上报

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| technologyId | 是 | long | 测试工艺ID |
| flag | 是 | string | "start" 或 "end" |

**响应**: 仅 code + message

---

### 8. 阶段开始/结束
- **接口**: `POST /client/cycleStageStartOrEnd`
- **说明**: 非初始化阶段的开始和结束上报

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| cycleStageId | 是 | long | 阶段ID |
| flag | 是 | string | "start" 或 "end" |

**响应**: 仅 code + message

---

## 四、测试项执行（6个）

### 9. 版本校验
- **接口**: `GET /client/checkTestItemVersion`
- **说明**: 运行测试前校验本地程序版本是否最新

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| cycleItemId | 是 | long | 测试项ID |
| version | 是 | string | 本地版本号 |

**响应 data (TestItemPO)**:

| 字段 | 类型 | 说明 |
|------|------|------|
| id | long | 主键ID |
| itemName | string | 测试项名称 |
| checkVersionResult | string | 版本是否匹配：0=不匹配, 1=匹配 |
| downLoadPath | string | 程序包下载路径 |
| extractHref | string | 解压路径 |
| callHref | string | 程序入口路径 |
| programName | string | 程序名称 |
| programType | int | 程序类型：1=exe, 2=bat |
| isAutoExecute | int | 是否自动执行：0=否, 1=是 |
| isRepeatTest | int | 支持重测：0=否, 1=是 |
| isOffline | int | 是否离线：0=离线, 1=在线 |
| tips | string | 知识库提示 |
| testItemParamList | array | 参数配置 [{key, name, value}] |
| testItemRuleList | array | 检测规则 [{ruleCode, ruleName, testStandard, errorInfo, isContinue, status, relationDateList}] |
| testItemDataList | array | 数据配置 [{key, name, valueType}] |
| testItemSpuRelationList | array | 适配机型 [{spuId, spuName}] |

---

### 10. 测试项开始
- **接口**: `POST /client/cycleItemStart`
- **说明**: 测试项开始执行时上报

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| cycleItemId | 是 | long | 测试项ID |
| flag | 是 | string | "start" |

**响应**: 仅 code + message

---

### 11. 测试项结束
- **接口**: `POST /client/cycleItemEnd`
- **说明**: 测试项执行完成时上报

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| cycleItemId | 是 | long | 测试项ID |
| flag | 是 | string | "end" |

**响应 data**: boolean

---

### 12. 查询测试项结果
- **接口**: `GET /client/queryCycleTestItemInfo`
- **说明**: 测试项完成后查询结果详情

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| cycleItemId | 是 | long | 测试项ID |

**响应 data (TestDeviceCycleItemVO)**:

| 字段 | 类型 | 说明 |
|------|------|------|
| id | long | 主键ID |
| itemId | long | 测试项ID |
| itemName | string | 测试项名称 |
| deviceCycleItemId | long | 生命周期测试项ID |
| deviceCycleStageId | long | 阶段生命周期ID |
| deviceCycleId | long | 设备生命周期ID |
| stageId | long | 阶段ID |
| technologyId | long | 工艺ID |
| callHref | string | 程序入口 |
| extractHref | string | 解压路径 |
| programType | int | 程序类型 |
| isOffline | int | 是否离线 |
| isRepeatTest | int | 是否支持重测 |
| productType | int | 产品类型 |
| status | int | 状态：0=未开始, 1=进行中, 2=已完成 |
| result | int | 结果：0=无, 1=通过, 2=失败 |
| tips | string | 知识库提示 |
| testDeviceCycleItemResultVOList | array | 规则结果（同上） |

---

### 13. 请求重测
- **接口**: `POST /client/cycleItemReTest`
- **说明**: 对失败的测试项请求重新测试

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| cycleItemId | 是 | long | 设备生命周期测试项ID |

**响应**: 仅 code + message

---

### 14. 查询测试项参数
- **接口**: `GET /client/queryTestItemParam`
- **说明**: 查询测试项的运行参数（传递给外部测试程序使用）

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| itemId | 是 | long | 测试项ID |

**响应 data**: `[{key, name, value}]`

---

## 五、数据上报（3个）

### 15. 上报测试数据
- **接口**: `POST /client/reportCycleTestData`
- **说明**: 外部测试程序调用，上报测试结果数据

**请求体**:

| 字段 | 类型 | 说明 |
|------|------|------|
| cycleItemId | long | 测试项ID |
| key | string | 数据key |
| testInfo | string | 测试信息 |
| value | object | 数据值 |

**响应**: 仅 code + message

---

### 16. 添加测试日志
- **接口**: `POST /client/addTestLog`
- **说明**: 老脚本测试日志记录

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| sn | 否 | string | 设备序列号 |
| status | 否 | int | 状态：0=开始, 1=关闭 |

**响应**: 仅 code + message

---

### 17. 添加扩展数据
- **接口**: `POST /client/addExtData`
- **说明**: Dump文件上传后的元数据上报

**请求体**:

| 字段 | 类型 | 说明 |
|------|------|------|
| sn | string | 设备序列号 |
| mac | string | MAC地址 |
| dataTime | string | 数据时间 |
| uploadTime | string | 上传时间 |
| type | int | 类型 |
| fileUrl | string | 文件URL（uploadFile返回的地址） |

**响应**: 仅 code + message

---

## 六、文件操作（1个）

### 18. 上传文件
- **接口**: `POST /file/uploadFile`
- **说明**: 上传Dump/蓝屏文件（multipart/form-data）

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| file | 是 | file | 文件内容 |

**响应 data**: string（文件URL，用于 addExtData 的 fileUrl）

---

## 七、周期管理（1个）

### 19. 关闭测试周期
- **接口**: `GET /open/closeDeviceCycle`
- **说明**: 维修模式下手动关闭测试周期

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| sn | 否 | string | 设备序列号 |
| flag | 否 | int | 固定传 1 |

**响应**: 仅 code + message

---

## 八、预留接口（待实现）

### 20. 检查扩展数据是否存在
- **接口**: `GET /client/existExtData`
- **说明**: 检查指定测试项的扩展数据是否已上报（避免重复上传Dump）

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| cycleItemId | 是 | int | 测试项ID |
| type | 是 | int | 数据类型 |

**响应 data**: boolean（true=已存在, false=不存在）

---

### 21. 检测项未及时关闭告警
- **接口**: `GET /client/notCloseAlert`
- **说明**: 检测项长时间未关闭时上报告警

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| param | 否 | string | 参数 |

**响应**: 仅 code + message

---

### 22. 获取客户端下载地址
- **接口**: `GET /client/queryClientDownloadInfo`
- **说明**: 获取客户端自身的新版本下载地址（用于客户端自动更新）

| 参数 | 必填 | 类型 | 说明 |
|------|:----:|------|------|
| sn | 是 | string | 设备序列号 |
| mac | 否 | string | MAC地址 |

**响应 data**:

| 字段 | 类型 | 说明 |
|------|------|------|
| clientFTPUrl | string | 客户端下载地址 |

---

## 接口统计

| 分类 | 数量 | 请求方式 |
|------|:----:|----------|
| 设备信息 | 3 | 2 GET + 1 POST |
| 测试计划 | 3 | 全部 GET |
| 阶段生命周期 | 2 | 全部 POST |
| 测试项执行 | 6 | 3 GET + 3 POST |
| 数据上报 | 3 | 全部 POST |
| 文件操作 | 1 | POST (multipart) |
| 周期管理 | 1 | GET |
| 预留接口 | 3 | 全部 GET |
| **合计** | **22** | **12 GET + 10 POST** |


