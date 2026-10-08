#include "../../src/apps/tracking_policy.hpp"

int main() {
    mig::Configuration profile;
    if (mig::app::hand_tracking_requested(profile)) {
        return 1;
    }
    profile.track_hands = true;
    if (!mig::app::hand_tracking_requested(profile)) {
        return 2;
    }
    profile.track_hands = false;
    if (mig::app::hand_tracking_requested(profile)) {
        return 3;
    }
}
