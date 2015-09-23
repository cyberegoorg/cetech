#pragma once

#include <cstring>
#include <cstdlib>

#include "rapidjson/document.h"

#include "common/asserts.h"
#include "common/cvar/cvar_types.h"
#include "common/crypto/murmur_hash.inl.h"
#include "common/log/log.h"
#include "common/string/stringid_types.h"

namespace cetech {
    extern CVar* _head;

    namespace cvar {
        CE_INLINE void set(CVar& cv, float value);
        CE_INLINE void set(CVar& cv, int value);
        CE_INLINE void set(CVar& cv, const char* str);

        CE_INLINE CVar* find(const char* name);

        CE_INLINE void load_from_json(const rapidjson::Document& document);

        CE_INLINE void dump_all();
    }

    namespace cvar_internal {
        CE_INLINE bool check_set_flags(const CVar& cv);

        CE_INLINE void force_set(CVar& cv, float value);
        CE_INLINE void force_set(CVar& cv, int value);
        CE_INLINE void force_set(CVar& cv, const char* str);
    }

    CVar::CVar(const char* name, const char* desc, CVarType type, uint32_t flags) : _next(nullptr), type(type), flags(
            flags) {
        strncpy(this->name, name, 512);
        strncpy(this->desc, desc, 1024);

        hash = murmur_hash_64(name, strlen(name), 22);

        f_max = 0.0f;
        f_min = 0.0f;

        if (_head == nullptr) {
            _head = this;
        } else {
            _next = _head;
            _head = this;
        }
    }

    CVar::CVar(const char* name, const char* desc, float value, uint32_t flags, float min, float max) : CVar(name,
                                                                                                             desc,
                                                                                                             CVAR_FLOAT,
                                                                                                             flags) {
        f_min = min;
        f_max = max;

        cvar_internal::force_set(*this, value);
    }

    CVar::CVar(const char* name, const char* desc, int value, uint32_t flags, int min, int max) : CVar(name,
                                                                                                       desc,
                                                                                                       CVAR_INT,
                                                                                                       flags) {
        i_min = min;
        i_max = max;

        cvar_internal::force_set(*this, value);
    }

    CVar::CVar(const char* name, const char* desc, const char* value, uint32_t flags) : CVar(name, desc, CVAR_STR,
                                                                                             flags) {
        cvar_internal::force_set(*this, value);
    }

    namespace cvar_internal {
        bool check_set_flags(const CVar& cv) {
            if (cv.flags & CVar::FLAG_CONST) {
                log::error("CVar", "Could not set cvar \"%s\". CVar is const.", cv.name);
                return false;
            }

            return true;
        }


        void force_set(CVar& cv, float value) {
            CE_ASSERT( cv.type == CVar::CVAR_FLOAT );

            if ((cv.f_min != cv.f_max) && (cv.f_max != 0)) {
                if ((value < cv.f_min) || (value > cv.f_max)) {
                    log::error("CVar", "Range error: %f <= (%f) <= %f", cv.f_min, value, cv.f_max);
                    return;
                }
            }

            cv.value_f = value;
        }

        void force_set(CVar& cv, int value) {
            CE_ASSERT( cv.type == CVar::CVAR_INT );

            if ((cv.f_min != cv.i_max) && (cv.i_max != 0)) {
                if ((value < cv.i_min) || (value > cv.i_max)) {
                    log::error("CVar", "Range error: %i <= (%i) <= %i", cv.i_min, value, cv.i_max);
                    return;
                }
            }

            cv.value_i = value;
        }

        void force_set(CVar& cv, const char* str) {
            CE_ASSERT( cv.type == CVar::CVAR_STR );

            if (cv.value_str) {
                free(cv.value_str);
            }

            cv.value_str = strdup(str);
            cv.str_len = strlen(str);
        }
    }

    namespace cvar {
        void set(CVar& cv, float value) {
            CE_ASSERT( cv.type == CVar::CVAR_FLOAT );

            if (!cvar_internal::check_set_flags(cv)) {
                return;
            }

            cvar_internal::force_set(cv, value);
        }

        void set(CVar& cv, int value) {
            CE_ASSERT( cv.type == CVar::CVAR_INT );

            if (!cvar_internal::check_set_flags(cv)) {
                return;
            }

            cvar_internal::force_set(cv, value);
        }

        void set(CVar& cv, const char* str) {
            CE_ASSERT( cv.type == CVar::CVAR_STR );

            if (!cvar_internal::check_set_flags(cv)) {
                return;
            }

            cvar_internal::force_set(cv, str);
        }

        CVar* find(const char* name) {
            const StringId64_t name_id = murmur_hash_64(name, strlen(name), 22);

            CVar* it = _head;
            while (it != nullptr) {
                if (it->hash != name_id) {
                    it = it->_next;
                    continue;
                }

                if (0 != strcmp(name, it->name)) {
                    continue;
                }

                return it;
            }

            return nullptr;
        }

        void load_from_json(const rapidjson::Document& document) {
            const rapidjson::Value& ar = document["cvars"];

            for (rapidjson::Value::ConstMemberIterator itr = ar.MemberBegin(); itr != ar.MemberEnd(); ++itr) {
                const rapidjson::Value& name = itr->name;
                const rapidjson::Value& value = itr->value;

                CVar* cvar = cvar::find(name.GetString());

                if (cvar == nullptr) {
                    log::error("cvar", "Undefined cvar \"%s\"", name.GetString());
                    continue;
                }

                /* INT */
                if (value.IsInt()) {
                    if (cvar->type != CVar::CVAR_INT) {
                        log::error("cvar", "Invalid type for cvar \"%s\".", name.GetString());
                        continue;
                    }

                    cvar_internal::force_set(*cvar, value.GetInt());

                    /* FLOAT */
                } else if (value.IsDouble()) {
                    if (cvar->type != CVar::CVAR_FLOAT) {
                        log::error("cvar", "Invalid type for cvar \"%s\".", name.GetString());
                        continue;
                    }

                    cvar_internal::force_set(*cvar, (float)value.GetDouble());

                    /* STR */
                } else if (value.IsString()) {
                    if (cvar->type != CVar::CVAR_STR) {
                        log::error("cvar", "Invalid type for cvar \"%s\".", name.GetString());
                        continue;
                    }

                    cvar_internal::force_set(*cvar, value.GetString());
                }
            }
        }

        void dump_all() {
            CVar* it = _head;

            uint32_t pos = 0;

            while (it != nullptr) {

                switch (it->type) {
                case CVar::CVAR_INT:
                    log::info("CVar.dump",
                              "[%u] name: %s  type: int  value: %i  desc: %s",
                              pos,
                              it->name,
                              it->value_i,
                              it->desc);
                    break;

                case CVar::CVAR_FLOAT:
                    log::info("CVar.dump",
                              "[%u] name: %s  type: float  value: %f  desc: %s",
                              pos,
                              it->name,
                              it->value_f,
                              it->desc);
                    break;

                case CVar::CVAR_STR:
                    log::info("CVar.dump",
                              "[%u] name: %s  type: str  value: \"%s\"  desc: %s",
                              pos,
                              it->name,
                              it->value_str,
                              it->desc);
                    break;
                }

                ++pos;
                it = it->_next;
            }
        }
    }
}