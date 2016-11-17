#ifndef P2P_COMMON_H
#define P2P_COMMON_H

#include <cstdint>
#include <string>

namespace p2p
{

/**
 * @brief Тип кода события
 */
using event_code_type = int;
/**
 * @brief Уникальный идентификатор события (в пределах объекта client)
 */
using message_id_type = uint64_t;
/**
 * @brief Глобально уникальный идентификатор пользователя (контакта);
 * формат строго не определён
 */
using friend_id_type = std::string;

}

#endif // P2P_COMMON_H
