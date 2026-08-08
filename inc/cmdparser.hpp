#ifndef CMDPARSER_HPP
#define CMDPARSER_HPP
#include <cstdint>


class CommandParser {
    public:
        enum class Command : uint8_t {
            Unknown = 0,
            GetTemp,
            ResetSystem,
            TestDisplay
        };

        struct CommandBlueprint {
            const char* commandText;
            uint32_t length;
            Command id;
        };

        Command parseCommand(char* command, uint32_t length);

        void generateTextForGetTemp(char* textBuff, int32_t temperature);
        void generateTextForCommand(char* textBuff, Command cmd);

    private:
        static constexpr CommandBlueprint m_commandArray[] = {
            {"get temp", 8, Command::GetTemp},
            {"system reset", 12, Command::ResetSystem}
        };
        static constexpr uint32_t m_commandArrayLength = sizeof(m_commandArray) / sizeof(m_commandArray[0]);
        
        //move it to command struct?
        static constexpr const char* m_responseTestDisplay = "Testing display...";
        static constexpr const char* m_responseResetSystem = "Resetting system...";
        static constexpr const char* m_responseUnknown = "Unknown command";

        static constexpr uint32_t ASCII_ALIGN = 48;

        void copyStringToBuff(char* dest, const char* src);

};

#endif