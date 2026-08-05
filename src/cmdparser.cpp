#include "cmdparser.hpp"

CommandParser::Command CommandParser::parseCommand(char* command, uint32_t length) {
    bool isMatch;
    for(int i = 0; i < m_commandArrayLength; ++i) {
        if(length == m_commandArray[i].length) {
            isMatch = true;
            for(int j = 0; j < length; ++j) {
                if(command[j] != m_commandArray[i].commandText[j]) {
                    isMatch = false;
                    break;
                }
            }
            if(isMatch) {
                return m_commandArray[i].id;
            }
        }
    }
    return CommandParser::Command::Unknown;
}

void CommandParser::copyStringToBuff(char* dest, const char* src) {
    uint32_t it = 0;
    while (src[it] != '\0') {
        dest[it] = src[it];
        it++;
    }
    dest[it] = '\0';
    return;
}

void CommandParser::generateTextForCommand(char* textBuff, CommandParser::Command cmd) {
    if(cmd == CommandParser::Command::Unknown) {
        CommandParser::copyStringToBuff(textBuff, CommandParser::m_responseUnknown);
    }
    if(cmd == CommandParser::Command::TestDisplay) {
        CommandParser::copyStringToBuff(textBuff, CommandParser::m_responseTestDisplay);
    }
    if(cmd == CommandParser::Command::ResetSystem) {
        CommandParser::copyStringToBuff(textBuff, CommandParser::m_responseResetSystem);
    }
    return;
}

void CommandParser::generateTextForGetTemp(char* textBuff, int32_t temperature) {
    textBuff[0] = static_cast<char>((temperature / 1000) + ASCII_ALIGN);
    textBuff[1] = static_cast<char>(((temperature / 100) % 10) + ASCII_ALIGN);
    textBuff[2] = '.';
    textBuff[3] = static_cast<char>(((temperature / 10) % 10) + ASCII_ALIGN);
    textBuff[4] = static_cast<char>((temperature % 10) + ASCII_ALIGN);
    textBuff[5] = '*';
    textBuff[6] = 'C';
}