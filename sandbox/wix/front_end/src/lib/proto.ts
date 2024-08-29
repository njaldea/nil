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
                    "MessageType_Wix": 0,
                    "MessageType_I64Update": 1,
                    "MessageType_TextUpdate": 2
                  }
                },
                "Range": {
                  "fields": {
                    "id": {
                      "type": "int32",
                      "id": 1
                    },
                    "value": {
                      "type": "int64",
                      "id": 2
                    },
                    "min": {
                      "type": "int64",
                      "id": 3
                    },
                    "max": {
                      "type": "int64",
                      "id": 4
                    },
                    "step": {
                      "type": "int32",
                      "id": 5
                    },
                    "label": {
                      "type": "string",
                      "id": 6
                    }
                  }
                },
                "Text": {
                  "fields": {
                    "id": {
                      "type": "int32",
                      "id": 1
                    },
                    "placeholder": {
                      "type": "string",
                      "id": 2
                    },
                    "value": {
                      "type": "string",
                      "id": 3
                    }
                  }
                },
                "I64Update": {
                  "fields": {
                    "id": {
                      "type": "int32",
                      "id": 1
                    },
                    "value": {
                      "type": "int64",
                      "id": 2
                    }
                  }
                },
                "Block": {
                  "fields": {
                    "label": {
                      "type": "string",
                      "id": 1
                    },
                    "widgets": {
                      "rule": "repeated",
                      "type": "Widget",
                      "id": 2
                    }
                  },
                  "nested": {
                    "Widget": {
                      "oneofs": {
                        "value": {
                          "oneof": [
                            "block",
                            "range",
                            "text"
                          ]
                        }
                      },
                      "fields": {
                        "block": {
                          "type": "Block",
                          "id": 1
                        },
                        "range": {
                          "type": "Range",
                          "id": 2
                        },
                        "text": {
                          "type": "Text",
                          "id": 3
                        }
                      }
                    }
                  }
                },
                "Wix": {
                  "fields": {
                    "blocks": {
                      "rule": "repeated",
                      "type": "Block",
                      "id": 1
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