#include <iostream>
#include <iomanip> // For pretty printing
using namespace std;

// --- Existing Interface and Filter (AccurateSMA) ---
template <typename T>
class IFilter {
public:
    virtual ~IFilter() {}
    virtual T filter(T input) = 0;
};

template <typename T, size_t N>
class AccurateSMA : public IFilter<T> {
public:
    T filter(T input) override {
        // Subtract the oldest value from the running sum
        _sum -= _buffer[_index];
        // Overwrite the oldest value with the new input
        _buffer[_index] = input;
        // Add the new input to the running sum
        _sum += input;
        // Advance the index (circularly)
        _index = (_index + 1) % N;
        // "Growing Window" logic: count samples until the buffer is full
        if (_count < N) _count++;
        // Return the average
        return _sum / static_cast<T>(_count);
    }
private:
    T _buffer[N] = {0}; 
    T _sum = 0; 
    size_t _index = 0; 
    size_t _count = 0;
};

// --- New Types ---
typedef float (*DataSource)();         // Function pointer for Input
typedef void (*ProcessingCallback)(float); // Function pointer for Output

// --- Data Provider Logic ---
// We use a static array and index to simulate a hardware buffer
static float rawSensors[] = {12.0, 12.2, 11.9, 14.5, 12.1, 12.0, 11.8, 15.2, 12.3, 12.1};
static size_t sensorIndex = 0;

float getNextVoltage() {
    if (sensorIndex < 10) {
        return rawSensors[sensorIndex++];
    }
    return -1.0f; // Signal "End of Data"
}

// --- The Core Processor ---
// Now it "Pulls" the data from the provider function
void processSensorStream(IFilter<float>& filter, DataSource source, ProcessingCallback outputAction) {
    if (!source || !outputAction) return;

    while (true) {
        float rawVal = source(); // Pulling data
        if (rawVal < 0) break;   // Exit condition

        float cleanVal = filter.filter(rawVal);
        outputAction(cleanVal);  // Executing action
    }
}

// --- Output Action ---
void checkVoltageThreshold(float value) {
    cout << "Filtered Output: " << value << (value < 11.5f ? " [LOW]" : "") << "\n";
}

// --- Main Function ---
int main() {
    AccurateSMA<float, 5> batteryFilter;

    cout << "--- Starting Data Stream (Pull Model) ---\n";

    // We pass the function POINTERS: getNextVoltage and checkVoltageThreshold
    processSensorStream(batteryFilter, getNextVoltage, checkVoltageThreshold);

    return 0;
}

