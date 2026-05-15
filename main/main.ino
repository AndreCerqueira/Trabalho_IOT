#include <Adafruit_NeoPixel.h>

const bool HAS_BUZZER = true;
const bool HAS_NEOPIXEL = false;
const bool HAS_LIVES = true;

const int PIN_COUNT = 4;
const int BUZZER_PIN = 5;
const int LIVES_COUNT = 3;
const int NEOPIXEL_PIN = A0;
const int NEOPIXEL_COUNT = 5;
const int POINTS_PER_LED = 2;
const int POINTS_PER_LEVEL = 10;

const int TONE_HIT_FREQ = 1000;
const int TONE_HIT_DUR = 100;
const int TONE_MISS_FREQ = 250;
const int TONE_MISS_DUR = 300;
const int TONE_COUNTDOWN_FREQ = 440;
const int TONE_COUNTDOWN_DUR = 200;
const int TONE_GO_FREQ = 880;
const int TONE_GO_DUR = 600;

const unsigned long COUNTDOWN_INTERVAL = 1000;
const int COUNTDOWN_TOTAL_TICKS = 3;

class Buzzer
{
private:
    int _pin;
    bool _isVictoryPlaying = false;
    int _victoryNoteIndex = 0;
    unsigned long _lastNoteTime = 0;
    const int _victoryNotes[6] = {523, 659, 784, 1047, 784, 1047};
    const int _victoryDurations[6] = {150, 150, 150, 200, 150, 400};

    bool _isDefeatPlaying = false;
    int _defeatNoteIndex = 0;
    const int _defeatNotes[6] = {330, 262, 196, 131, 98, 65};
    const int _defeatDurations[6] = {150, 150, 150, 150, 150, 500};

public:
    void Initialize(int pin)
    {
        _pin = pin;
        pinMode(_pin, OUTPUT);
    }

    void PlayCountdownTick()
    {
        if (HAS_BUZZER)
            tone(_pin, TONE_COUNTDOWN_FREQ, TONE_COUNTDOWN_DUR);
    }

    void PlayCountdownGo()
    {
        if (HAS_BUZZER)
            tone(_pin, TONE_GO_FREQ, TONE_GO_DUR);
    }

    void PlayHit()
    {
        if (HAS_BUZZER)
            tone(_pin, TONE_HIT_FREQ, TONE_HIT_DUR);
    }

    void PlayMiss()
    {
        if (HAS_BUZZER)
            tone(_pin, TONE_MISS_FREQ, TONE_MISS_DUR);
    }

    void StartVictory(unsigned long currentTime)
    {
        _isVictoryPlaying = true;
        _victoryNoteIndex = 0;
        _lastNoteTime = currentTime;
        
        if (HAS_BUZZER)
            tone(_pin, _victoryNotes[0], _victoryDurations[0]);
    }

    void UpdateVictory(unsigned long currentTime)
    {
        if (!_isVictoryPlaying)
            return;

        if (_victoryNoteIndex >= 6)
            return;

        if (currentTime - _lastNoteTime < (unsigned long)_victoryDurations[_victoryNoteIndex])
            return;

        _lastNoteTime = currentTime;
        _victoryNoteIndex++;

        if (_victoryNoteIndex < 6 && HAS_BUZZER)
            tone(_pin, _victoryNotes[_victoryNoteIndex], _victoryDurations[_victoryNoteIndex]);
    }

    void StartDefeat(unsigned long currentTime)
    {
        _isDefeatPlaying = true;
        _defeatNoteIndex = 0;
        _lastNoteTime = currentTime;
        
        if (HAS_BUZZER)
            tone(_pin, _defeatNotes[0], _defeatDurations[0]);
    }

    void UpdateDefeat(unsigned long currentTime)
    {
        if (!_isDefeatPlaying)
            return;

        if (_defeatNoteIndex >= 6)
            return;

        if (currentTime - _lastNoteTime < (unsigned long)_defeatDurations[_defeatNoteIndex])
            return;

        _lastNoteTime = currentTime;
        _defeatNoteIndex++;

        if (_defeatNoteIndex < 6 && HAS_BUZZER)
            tone(_pin, _defeatNotes[_defeatNoteIndex], _defeatDurations[_defeatNoteIndex]);
    }
};

class Mole
{
private:
    int _buttonPin;
    int _ledPin;

public:
    void Initialize(int buttonPin, int ledPin)
    {
        _buttonPin = buttonPin;
        _ledPin = ledPin;
        pinMode(_buttonPin, INPUT_PULLUP);
        pinMode(_ledPin, OUTPUT);
    }

    bool IsPressed()
    {
        return digitalRead(_buttonPin) == LOW;
    }

    bool IsReleased()
    {
        return digitalRead(_buttonPin) == HIGH;
    }

    void Show()
    {
        digitalWrite(_ledPin, HIGH);
    }

    void Hide()
    {
        digitalWrite(_ledPin, LOW);
    }
};

class LifeManager
{
private:
    int _ledPins[LIVES_COUNT];
    int _currentLives;
    unsigned long _lastBlinkTime = 0;
    bool _isBlinkActive = false;
    unsigned long _blinkInterval = 500;

public:
    void Initialize(const int pins[])
    {
        _currentLives = LIVES_COUNT;

        if (!HAS_LIVES)
            return;
        
        for (auto i = 0; i < LIVES_COUNT; i++)
        {
            _ledPins[i] = pins[i];
            pinMode(_ledPins[i], OUTPUT);
            digitalWrite(_ledPins[i], HIGH);
        }
    }

    void LoseLife()
    {
        if (_currentLives <= 0)
            return;

        _currentLives--;

        if (HAS_LIVES)
            digitalWrite(_ledPins[_currentLives], LOW);
    }

    bool IsGameOver()
    {
        return _currentLives == 0;
    }

    void UpdateGameOverBlink(unsigned long currentTime)
    {
        if (!HAS_LIVES)
            return;

        if (currentTime - _lastBlinkTime < _blinkInterval)
            return;

        _isBlinkActive = !_isBlinkActive;
        _lastBlinkTime = currentTime;

        for (auto i = 0; i < LIVES_COUNT; i++)
            digitalWrite(_ledPins[i], _isBlinkActive ? HIGH : LOW);
    }
};

class ScoreManager
{
private:
    Adafruit_NeoPixel _strip;
    int _currentScore;
    int _currentLevel;
    unsigned long _lastCascadeTime;
    int _cascadeIndex;
    const unsigned long _cascadeInterval = 100;
    bool _isVictory;

public:
    ScoreManager() : _strip(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800)
    {
    }

    void Initialize()
    {
        if (HAS_NEOPIXEL)
        {
            _strip.begin();
            _strip.clear();
            _strip.show();
        }
        
        _currentScore = 0;
        _currentLevel = 1;
        _isVictory = false;
        _cascadeIndex = 0;
        _lastCascadeTime = 0;
    }

    int GetLevel()
    {
        return _currentLevel;
    }

    void AddScore()
    {
        if (_currentScore >= POINTS_PER_LEVEL)
            return;

        _currentScore++;

        if (HAS_NEOPIXEL)
        {
            uint32_t color;
            
            if (_currentLevel == 1)
                color = _strip.Color(0, 255, 0);
            else if (_currentLevel == 2)
                color = _strip.Color(0, 0, 255);
            else
                color = _strip.Color(255, 0, 0);

            auto litLeds = _currentScore / POINTS_PER_LED;
            
            for (auto i = 0; i < litLeds; i++)
                _strip.setPixelColor(i, color);

            _strip.show();
        }

        if (_currentScore >= POINTS_PER_LEVEL)
        {
            if (_currentLevel < 3)
            {
                _currentLevel++;
                _currentScore = 0;
                
                if (HAS_NEOPIXEL)
                {
                    _strip.clear();
                    _strip.show();
                }
            }
            else
            {
                _isVictory = true;
            }
        }
    }

    bool IsGameWon()
    {
        return _isVictory;
    }

    void UpdateVictoryAnimation(unsigned long currentTime)
    {
        if (!HAS_NEOPIXEL)
            return;

        if (currentTime - _lastCascadeTime < _cascadeInterval)
            return;

        _lastCascadeTime = currentTime;
        _strip.clear();

        for (auto i = 0; i <= _cascadeIndex; i++)
            _strip.setPixelColor(i, _strip.Color(random(0, 256), random(0, 256), random(0, 256)));

        _strip.show();
        _cascadeIndex++;

        if (_cascadeIndex >= NEOPIXEL_COUNT)
            _cascadeIndex = 0;
    }
};

class GameManager
{
private:
    Mole _moles[PIN_COUNT];
    Buzzer _buzzer;
    LifeManager _lifeManager;
    ScoreManager _scoreManager;
    
    bool _activeMoles[PIN_COUNT];
    bool _hasActiveMoles = false;
    
    unsigned long _lastMoleTime = 0;
    unsigned long _moleDuration = 1000;
    unsigned long _pauseDuration = 500;
    
    bool _hasWon = false;
    bool _hasLost = false;
    
    bool _isStarting = true;
    bool _hasInitializedCountdown = false;
    int _countdownTicks = 0;
    unsigned long _lastTickTime = 0;

public:
    void Initialize(const int buttonPins[], const int ledPins[], int buzzerPin, const int lifePins[])
    {
        for (auto i = 0; i < PIN_COUNT; i++)
            _moles[i].Initialize(buttonPins[i], ledPins[i]);

        for (auto i = 0; i < PIN_COUNT; i++)
            _activeMoles[i] = false;

        _buzzer.Initialize(buzzerPin);
        _lifeManager.Initialize(lifePins);
        _scoreManager.Initialize();
        randomSeed(analogRead(1));
        
        _hasWon = false;
        _hasLost = false;
        _hasActiveMoles = false;
        _isStarting = true;
        _hasInitializedCountdown = false;
        _countdownTicks = 0;
        
        UpdateDifficulty();
    }

    void Update()
    {
        auto currentTime = millis();

        if (_isStarting)
        {
            HandleStartup(currentTime);
            return;
        }

        if (_hasWon)
        {
            _scoreManager.UpdateVictoryAnimation(currentTime);
            _buzzer.UpdateVictory(currentTime);
            return;
        }

        if (_lifeManager.IsGameOver())
        {
            if (!_hasLost)
            {
                _hasLost = true;
                _buzzer.StartDefeat(currentTime);
            }

            _buzzer.UpdateDefeat(currentTime);
            _lifeManager.UpdateGameOverBlink(currentTime);
            return;
        }

        if (!_hasActiveMoles)
        {
            HandleSpawn(currentTime);
            return;
        }

        HandleActiveMole(currentTime);
    }

private:
    void UpdateDifficulty()
    {
        auto level = _scoreManager.GetLevel();
        
        if (level == 1)
        {
            _moleDuration = 1000;
            _pauseDuration = 500;
        }
        else if (level == 2)
        {
            _moleDuration = 800;
            _pauseDuration = 400;
        }
        else if (level == 3)
        {
            _moleDuration = 600;
            _pauseDuration = 300;
        }
    }

    void HandleStartup(unsigned long currentTime)
    {
        if (!_hasInitializedCountdown)
        {
            _buzzer.PlayCountdownTick();
            _countdownTicks = 1;
            _lastTickTime = currentTime;
            _hasInitializedCountdown = true;
            return;
        }

        if (currentTime - _lastTickTime < COUNTDOWN_INTERVAL)
            return;

        _lastTickTime = currentTime;

        if (_countdownTicks < COUNTDOWN_TOTAL_TICKS)
        {
            _buzzer.PlayCountdownTick();
            _countdownTicks++;
            return;
        }

        _buzzer.PlayCountdownGo();
        _isStarting = false;
        _lastMoleTime = currentTime;
    }

    void HandleSpawn(unsigned long currentTime)
    {
        if (currentTime - _lastMoleTime < _pauseDuration)
            return;

        for (auto i = 0; i < PIN_COUNT; i++)
            if (_moles[i].IsPressed())
                return;

        UpdateDifficulty();
        
        auto level = _scoreManager.GetLevel();
        auto molesToSpawn = 1;

        if (level == 2)
            molesToSpawn = 2;
        else if (level == 3)
            molesToSpawn = random(1, PIN_COUNT + 1);

        for (auto i = 0; i < PIN_COUNT; i++)
            _activeMoles[i] = false;

        auto spawned = 0;
        
        while (spawned < molesToSpawn)
        {
            auto idx = random(PIN_COUNT);
            
            if (!_activeMoles[idx])
            {
                _activeMoles[idx] = true;
                _moles[idx].Show();
                spawned++;
            }
        }

        _hasActiveMoles = true;
        _lastMoleTime = currentTime;
    }

    void HandleActiveMole(unsigned long currentTime)
    {
        if (currentTime - _lastMoleTime >= _moleDuration)
        {
            RegisterMiss(currentTime);
            return;
        }

        auto isAllRequiredPressed = true;
        auto isAnyWrongPressed = false;

        for (auto i = 0; i < PIN_COUNT; i++)
        {
            auto isPressed = _moles[i].IsPressed();

            if (_activeMoles[i] && !isPressed)
                isAllRequiredPressed = false;

            if (!_activeMoles[i] && isPressed)
                isAnyWrongPressed = true;
        }

        if (isAnyWrongPressed)
        {
            RegisterMiss(currentTime);
            return;
        }

        if (isAllRequiredPressed)
        {
            RegisterHit(currentTime);
            return;
        }
    }

    void RegisterHit(unsigned long currentTime)
    {
        _scoreManager.AddScore();
        
        if (_scoreManager.IsGameWon())
        {
            _hasWon = true;
            _buzzer.StartVictory(currentTime);
            HideCurrentMoles(currentTime);
            return;
        }

        _buzzer.PlayHit();
        HideCurrentMoles(currentTime);
    }

    void RegisterMiss(unsigned long currentTime)
    {
        _buzzer.PlayMiss();
        _lifeManager.LoseLife();
        HideCurrentMoles(currentTime);
    }

    void HideCurrentMoles(unsigned long currentTime)
    {
        for (auto i = 0; i < PIN_COUNT; i++)
        {
            _moles[i].Hide();
            _activeMoles[i] = false;
        }
            
        _hasActiveMoles = false;
        _lastMoleTime = currentTime;
    }
};

const int BUTTON_PINS[] = {4, 3, 2, 1};
const int LED_PINS[] = {13, 12, 11, 10};
const int LIFE_PINS[] = {6, 7, 8};

GameManager gameManager;

void setup()
{
    gameManager.Initialize(BUTTON_PINS, LED_PINS, BUZZER_PIN, LIFE_PINS);
}

void loop()
{
    gameManager.Update();
}