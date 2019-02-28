#ifndef _SCHEMA_DEFINE_H_
#define _SCHEMA_DEFINE_H_

namespace constants {

const char* schema = R"(
{
    "$schema": "http://json-schema.org/draft-04/schema#",
    "title": "Instrument config",
    "type": "object",
    "properties": {
        "instruments": {
            "type": "object",
            "patternProperties": {
                "^.*$": {
                    "type": "object",
                    "properties": {
                        "destination": {
                            "type": "string"
                        },
                        "mode": {
                            "type": "string"
                        }
                    },
                    "required": [
                        "destination",
                        "mode"
                    ]
                }
            }
        },
        "modes": {
            "type": "object",
            "patternProperties": {
                "^mode": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "begin": {
                                "type": "string"
                            },
                            "end": {
                                "type": "string"
                            }
                        },
                        "required": [
                            "begin",
                            "end"
                        ]
                    }
                }
            }
        }
    },
    "required": [
        "instruments",
        "modes"
    ]
}
)";

}  // namespace constants

#endif  // _SCHEMA_DEFINE_H_
