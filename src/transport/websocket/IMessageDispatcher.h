#pragma once

#include <string>

struct IMessageDispatcher
{
    // TODO: Likely need to enforce FIFO on message sending through this socket, perhaps multi-producer-single-consumer?
    // As-is, no guarantee exists, so a caller could send multiple messages and have them dispatched out-of-order;
    // I could see this posing a problem for notifications, e.g. object-added and object-changed switching leads
    // to "invalid" object-change notification (referring to non-existent object)
    // If the lock preserves ordering of attempted acquisition, then we should be good
    virtual bool SendMessage(std::string message) = 0;
};