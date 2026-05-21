#include <Adafruit_NeoPixel.h>

const bool HAS_BUZZER = true;
const bool HAS_LIVES = true;

const int PIN_COUNT = 4;
const int BUZZER_PIN = 5;
const int RESET_PIN = 10;
const int LIVES_COUNT = 3;

const int MOLES_NEOPIXEL_PIN = A1;
const int LIVES_NEOPIXEL_PIN = A2;
const int SCORE_NEOPIXEL_PIN = A0;
const int RANDOM_SEED_PIN = A5;

const int SCORE_NEOPIXEL_COUNT = 5;
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

const int VICTORY_NOTES[6] = { 523, 659, 784, 1047, 784, 1047 };
const int VICTORY_DURATIONS[6] = { 150, 150, 150, 200, 150, 400 };
const int VICTORY_NOTES_COUNT = 6;

const int DEFEAT_NOTES[6] = { 330, 262, 196, 131, 98, 65 };
const int DEFEAT_DURATIONS[6] = { 150, 150, 150, 150, 150, 500 };
const int DEFEAT_NOTES_COUNT = 6;

const unsigned long BLINK_INTERVAL = 500;
const unsigned long CASCADE_INTERVAL = 100;

const int COLOR_MAX = 255;
const int COLOR_MIN = 0;
const int RANDOM_COLOR_MAX = 256;

const int LEVEL_ONE = 1;
const int LEVEL_TWO = 2;
const int LEVEL_THREE = 3;

const unsigned long LEVEL_ONE_DURATION = 1000;
const unsigned long LEVEL_ONE_PAUSE = 500;
const unsigned long LEVEL_TWO_DURATION = 800;
const unsigned long LEVEL_TWO_PAUSE = 400;
const unsigned long LEVEL_THREE_DURATION = 600;
const unsigned long LEVEL_THREE_PAUSE = 300;

const int MOLES_LEVEL_TWO = 2;

namespace Game
{
    class Buzzer
    {
    private:
        int _pin;
        bool _isVictoryPlaying = false;
        int _victoryNoteIndex = 0;
        unsigned long _lastNoteTime = 0;

        bool _isDefeatPlaying = false;
        int _defeatNoteIndex = 0;

    public:
        void Initialize(int pin)
        {
            _pin = pin;
            pinMode(_pin, OUTPUT);
        }

        void Reset()
        {
            _isVictoryPlaying = false;
            _isDefeatPlaying = false;
            noTone(_pin);
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
                tone(_pin, VICTORY_NOTES[0], VICTORY_DURATIONS[0]);
        }

        void UpdateVictory(unsigned long currentTime)
        {
            if (!_isVictoryPlaying)
                return;

            if (_victoryNoteIndex >= VICTORY_NOTES_COUNT)
                return;

            if (currentTime - _lastNoteTime < (unsigned long)VICTORY_DURATIONS[_victoryNoteIndex])
                return;

            _lastNoteTime = currentTime;
            _victoryNoteIndex++;

            if (_victoryNoteIndex < VICTORY_NOTES_COUNT && HAS_BUZZER)
                tone(_pin, VICTORY_NOTES[_victoryNoteIndex], VICTORY_DURATIONS[_victoryNoteIndex]);
        }

        void StartDefeat(unsigned long currentTime)
        {
            _isDefeatPlaying = true;
            _defeatNoteIndex = 0;
            _lastNoteTime = currentTime;
            
            if (HAS_BUZZER)
                tone(_pin, DEFEAT_NOTES[0], DEFEAT_DURATIONS[0]);
        }

        void UpdateDefeat(unsigned long currentTime)
        {
            if (!_isDefeatPlaying)
                return;

            if (_defeatNoteIndex >= DEFEAT_NOTES_COUNT)
                return;

            if (currentTime - _lastNoteTime < (unsigned long)DEFEAT_DURATIONS[_defeatNoteIndex])
                return;

            _lastNoteTime = currentTime;
            _defeatNoteIndex++;

            if (_defeatNoteIndex < DEFEAT_NOTES_COUNT && HAS_BUZZER)
                tone(_pin, DEFEAT_NOTES[_defeatNoteIndex], DEFEAT_DURATIONS[_defeatNoteIndex]);
        }
    };

    class Mole
    {
    private:
        int _buttonPin;

    public:
        void Initialize(int buttonPin)
        {
            _buttonPin = buttonPin;
            pinMode(_buttonPin, INPUT);
        }

        bool IsPressed()
        {
            return digitalRead(_buttonPin) == HIGH;
        }
    };

    class LifeManager
    {
    private:
        Adafruit_NeoPixel _strip;
        int _currentLives;
        unsigned long _lastBlinkTime = 0;
        bool _isBlinkActive = false;

    public:
        LifeManager() : _strip(LIVES_COUNT, LIVES_NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800)
        {
        }

        void Initialize()
        {
            if (HAS_LIVES)
                _strip.begin();
        }

        void Reset()
        {
            _currentLives = LIVES_COUNT;
            _isBlinkActive = false;

            if (!HAS_LIVES)
                return;

            _strip.clear();
            // As luzes de vida começam apagadas para o efeito de contagem
            _strip.show();
        }
        
        // Nova função para acender uma luz de vida específica
        void TurnOnLifeLed(int index)
        {
            if (!HAS_LIVES || index < 0 || index >= LIVES_COUNT)
                return;

            _strip.setPixelColor(index, _strip.Color(COLOR_MAX, COLOR_MIN, COLOR_MIN));
            _strip.show();
        }

        void LoseLife()
        {
            if (_currentLives <= 0)
                return;

            _currentLives--;

            if (HAS_LIVES)
            {
                _strip.setPixelColor(_currentLives, _strip.Color(COLOR_MIN, COLOR_MIN, COLOR_MIN));
                _strip.show();
            }
        }

        bool IsGameOver()
        {
            return _currentLives == 0;
        }

        void UpdateGameOverBlink(unsigned long currentTime)
        {
            if (!HAS_LIVES)
                return;

            if (currentTime - _lastBlinkTime < BLINK_INTERVAL)
                return;

            _isBlinkActive = !_isBlinkActive;
            _lastBlinkTime = currentTime;

            for (auto i = 0; i < LIVES_COUNT; i++)
                _strip.setPixelColor(i, _isBlinkActive ? _strip.Color(COLOR_MAX, COLOR_MIN, COLOR_MIN) : _strip.Color(COLOR_MIN, COLOR_MIN, COLOR_MIN));
                
            _strip.show();
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
        bool _isVictory;

    public:
        ScoreManager() : _strip(SCORE_NEOPIXEL_COUNT, SCORE_NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800)
        {
        }

        void Initialize()
        {
            _strip.begin();
        }

        void Reset()
        {
            _currentScore = 0;
            _currentLevel = LEVEL_ONE;
            _isVictory = false;
            _cascadeIndex = 0;
            _lastCascadeTime = 0;
            
            _strip.clear();
            _strip.show();
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

            uint32_t color;
            
            // Novas cores para os níveis
            if (_currentLevel == LEVEL_ONE)
                color = _strip.Color(COLOR_MIN, COLOR_MAX, COLOR_MIN); // Verde
            else if (_currentLevel == LEVEL_TWO)
                color = _strip.Color(COLOR_MAX, COLOR_MAX, COLOR_MIN); // Amarelo
            else
                color = _strip.Color(COLOR_MAX, COLOR_MIN, COLOR_MAX); // Magenta / Roxo

            auto litLeds = _currentScore / POINTS_PER_LED;
            
            for (auto i = 0; i < litLeds; i++)
                _strip.setPixelColor(i, color);

            _strip.show();

            if (_currentScore >= POINTS_PER_LEVEL)
            {
                if (_currentLevel < LEVEL_THREE)
                {
                    _currentLevel++;
                    _currentScore = 0;
                    
                    _strip.clear();
                    _strip.show();
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
            if (currentTime - _lastCascadeTime < CASCADE_INTERVAL)
                return;

            _lastCascadeTime = currentTime;
            _strip.clear();

            for (auto i = 0; i <= _cascadeIndex; i++)
                _strip.setPixelColor(i, _strip.Color(random(COLOR_MIN, RANDOM_COLOR_MAX), random(COLOR_MIN, RANDOM_COLOR_MAX), random(COLOR_MIN, RANDOM_COLOR_MAX)));

            _strip.show();
            _cascadeIndex++;

            if (_cascadeIndex >= SCORE_NEOPIXEL_COUNT)
                _cascadeIndex = 0;
        }
    };

    class GameManager
    {
    private:
        Mole _moles[PIN_COUNT];
        Adafruit_NeoPixel _molesStrip;
        Buzzer _buzzer;
        LifeManager _lifeManager;
        ScoreManager _scoreManager;
        
        int _resetPin;
        bool _activeMoles[PIN_COUNT];
        bool _hasActiveMoles = false;
        
        unsigned long _lastMoleTime = 0;
        unsigned long _moleDuration = LEVEL_ONE_DURATION;
        unsigned long _pauseDuration = LEVEL_ONE_PAUSE;
        
        bool _hasWon = false;
        bool _hasLost = false;
        
        bool _isStarting = true;
        bool _hasInitializedCountdown = false;
        int _countdownTicks = 0;
        unsigned long _lastTickTime = 0;

    public:
        GameManager() : _molesStrip(PIN_COUNT, MOLES_NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800)
        {
        }

        void Initialize(const int buttonPins[], int buzzerPin, int resetPin)
        {
            _resetPin = resetPin;
            pinMode(_resetPin, INPUT);

            randomSeed(analogRead(RANDOM_SEED_PIN));

            _molesStrip.begin();

            for (auto i = 0; i < PIN_COUNT; i++)
                _moles[i].Initialize(buttonPins[i]);

            _buzzer.Initialize(buzzerPin);
            _lifeManager.Initialize();
            _scoreManager.Initialize();
            
            Reset();
        }

        void Reset()
        {
            _buzzer.Reset();
            _lifeManager.Reset();
            _scoreManager.Reset();

            _molesStrip.clear();
            _molesStrip.show();

            for (auto i = 0; i < PIN_COUNT; i++)
                _activeMoles[i] = false;

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
            if (digitalRead(_resetPin) == HIGH)
            {
                Reset();
                return;
            }

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
            
            if (level == LEVEL_ONE)
            {
                _moleDuration = LEVEL_ONE_DURATION;
                _pauseDuration = LEVEL_ONE_PAUSE;
            }
            else if (level == LEVEL_TWO)
            {
                _moleDuration = LEVEL_TWO_DURATION;
                _pauseDuration = LEVEL_TWO_PAUSE;
            }
            else if (level == LEVEL_THREE)
            {
                _moleDuration = LEVEL_THREE_DURATION;
                _pauseDuration = LEVEL_THREE_PAUSE;
            }
        }

        void HandleStartup(unsigned long currentTime)
        {
            if (!_hasInitializedCountdown)
            {
                _buzzer.PlayCountdownTick();
                _lifeManager.TurnOnLifeLed(0); // Acende a primeira luz
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
                _lifeManager.TurnOnLifeLed(_countdownTicks); // Acende a 2ª e a 3ª luz em sequência
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

            if (level == LEVEL_TWO)
                molesToSpawn = MOLES_LEVEL_TWO;
            else if (level == LEVEL_THREE)
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
                    _molesStrip.setPixelColor(idx, _molesStrip.Color(COLOR_MIN, COLOR_MIN, COLOR_MAX));
                    spawned++;
                }
            }

            _molesStrip.show();
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
                _molesStrip.setPixelColor(i, _molesStrip.Color(COLOR_MIN, COLOR_MIN, COLOR_MIN));
                _activeMoles[i] = false;
            }
                
            _molesStrip.show();
            _hasActiveMoles = false;
            _lastMoleTime = currentTime;
        }
    };
}

const int BUTTON_PINS[] = {13, 12, 6, 11};

Game::GameManager gameManager;

void setup()
{
    gameManager.Initialize(BUTTON_PINS, BUZZER_PIN, RESET_PIN);
}

void loop()
{
    gameManager.Update();
}