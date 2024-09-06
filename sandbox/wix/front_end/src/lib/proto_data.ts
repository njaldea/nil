export const proto_data = 
{
  "options": {
    "syntax": "proto3",
    "optimize_for": "LITE_RUNTIME"
  },
  "nested": {
    "nil": {
      "nested": {
        "wix": {
          "nested": {
            "proto": {
              "nested": {
                "MessageType": {
                  "values": {
                    "MessageType_MarkupRequest": 0,
                    "MessageType_MarkupResponse": 1,
                    "MessageType_BindingRequest": 2,
                    "MessageType_BindingResponse": 3,
                    "MessageType_BindingUpdate": 4,
                    "MessageType_FileRequest": 5,
                    "MessageType_FileResponse": 6
                  }
                },
                "MarkupResponse": {
                  "fields": {
                    "components": {
                      "rule": "repeated",
                      "type": "string",
                      "id": 1
                    }
                  }
                },
                "Binding": {
                  "oneofs": {
                    "value": {
                      "oneof": [
                        "valueI64",
                        "valueStr"
                      ]
                    }
                  },
                  "fields": {
                    "tag": {
                      "type": "string",
                      "id": 1
                    },
                    "valueI64": {
                      "type": "int64",
                      "id": 2
                    },
                    "valueStr": {
                      "type": "string",
                      "id": 3
                    }
                  }
                },
                "BindingGroup": {
                  "fields": {
                    "bindings": {
                      "rule": "repeated",
                      "type": "Binding",
                      "id": 1
                    }
                  }
                },
                "BindingResponse": {
                  "fields": {
                    "info": {
                      "rule": "repeated",
                      "type": "BindingGroup",
                      "id": 1
                    }
                  }
                },
                "FileRequest": {
                  "fields": {
                    "target": {
                      "type": "string",
                      "id": 1
                    }
                  }
                },
                "FileResponse": {
                  "fields": {
                    "target": {
                      "type": "string",
                      "id": 1
                    },
                    "content": {
                      "type": "string",
                      "id": 3
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}