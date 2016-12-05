// Experiment in using FRP-like techniques for embedded systems,
// applied to some LED animations

#include <stdint.h>

#ifdef DO_SIMULATION
#include <stdio.h>
#include <vector>
#include <string>
#endif

#ifdef HAVE_JSON11
#include <json11.hpp>
#include <json11.cpp>
#endif

struct RgbColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
#ifdef HAVE_JSON11
    json11::Json to_json() const {
    using namespace json11;
    return Json::object {
            {"r", r},
            {"g", g},
            {"b", b},
    };
}
#endif
};

// Anything that may influence the system (cause state to change)
struct Input {
    long timeMs;
    int periodMs;
    RgbColor color;
#ifdef HAVE_JSON11
    json11::Json to_json() const {
    using namespace json11;
    return Json::object {
            {"timeMs", timeMs},
            {"periodMs", periodMs},
            {"color", color.to_json()},
    };
}
#endif
};

// State of system
struct State {
    RgbColor ledColor;
};

struct Config {
    bool _;
};


State
nextState(const Input &input, const State& previous) {
    // FIXME: the integer math here is not sound, overflows or something
    const int period = input.periodMs;
    const long pos = input.timeMs % period;
    
    State s = previous;
    
    const long time = (input.timeMs) ? input.timeMs : 1; // prevent division by zero
    const long mod = (pos*255) / time;
    s.ledColor = {
      (uint8_t)(input.color.r*mod/255),
      (uint8_t)(input.color.g*mod/255),
      (uint8_t)(input.color.b*mod/255)
    };

    return s;
}

#ifdef DO_SIMULATION
// Render using console Truecolor https://gist.github.com/XVilka/8346728
void
colorRenderTerminal(RgbColor color, const std::string &text) {
    const int background = 2;
    printf("\x1b[38;%d;%d;%d;%dm%s\x1b[0m",
           background, color.r, color.g, color.b, text.c_str());

}

bool
realizeState(const State& state, const Config &config) {
    // TODO: implement for hardware
    // TODO: implement via MsgFlo, sending MQTT message to HW-unit
    colorRenderTerminal(state.ledColor, "(#####)\n");
    return true;
}

// TODO: serialize inputs and outputs as a Flowtrace

int
main(int argc, char *argv[]) {

    std::vector<Input> history;

    const int simulationInterval = 100;
    const int simulationTime = 7*1000;

    int currentTime = 0;
    State previousState;
    Input in = { currentTime, 2100, { 255, 255, 0 } };
#ifdef HAVE_JSON11
    printf("%s", in.to_json().dump().c_str());
#endif
    Config config;
    while (currentTime < simulationTime) {
        in.timeMs = currentTime;
        const State state = nextState(in, previousState);
        realizeState(state, config);
        history.push_back(in);
        previousState = state;
        currentTime += simulationInterval;
    }

}
#endif
