#pragma once
#ifndef GAMECLASSES_H
#define GAMECLASSES_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <fstream>

using namespace std;




string currentPlayerName = "Player";




/// purpose: animates text with a typewriter effect for menus and intros.
/// parameters: configured with string content, fonts, and colors; update uses dt in seconds.
/// return: reports completion status for state transitions.
class TypewriterText {
private:
    sf::Text text;
    sf::Font font;
    string full;
    string shown;
    float speed;
    float timer;
    bool complete;
    bool active;

public:
    TypewriterText() {
        speed = 0.05f;
        timer = 0.0f;
        complete = false;
        active = false;
    }

    void setup(const string& s, const sf::Font& f, unsigned int size, sf::Color col) {
        full = s;
        shown = "";
        font = f;
        text.setFont(font);
        text.setCharacterSize(size);
        text.setFillColor(col);
        complete = false;
        timer = 0.0f;

        sf::FloatRect b = text.getLocalBounds();
        text.setOrigin(b.left + b.width / 2.0f, b.top + b.height / 2.0f);
    }

    void start() {
        shown = "";
        complete = false;
        active = true;
        timer = 0.0f;
        text.setString("");
    }

    void update(float dt) {
        if (!active || complete) return;

        timer += dt;
        if (timer >= speed) {
            timer = 0.0f;

            if (shown.length() < full.length()) {
                shown += full[shown.length()];
                text.setString(shown);

                sf::FloatRect b = text.getLocalBounds();
                text.setOrigin(b.left + b.width / 2.0f, b.top + b.height / 2.0f);
            }
            else {
                complete = true;
            }
        }
    }

    void draw(sf::RenderWindow& win) {
        if (active) win.draw(text);
    }

    void setPosition(float x, float y) {
        text.setPosition(x, y);
    }

    bool isDone() const {
        return complete;
    }

    void setActive(bool a) {
        active = a;
    }
};




/// purpose: represents a stylized menu option with animated selection states.
/// parameters: set up with label, font, placement, and theme color; update uses dt for transitions.
/// return: n/a.
class MenuItem {
private:
    sf::Text text;
    sf::RectangleShape box;

    sf::Color idleTextColor;
    sf::Color selTextColor;
    sf::Color idleBoxColor;
    sf::Color selBoxColor;
    sf::Color idleOutlineColor;
    sf::Color selOutlineColor;

    bool selected;
    float transition;

    static sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t) {
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        return sf::Color(
            (sf::Uint8)(a.r + (b.r - a.r) * t),
            (sf::Uint8)(a.g + (b.g - a.g) * t),
            (sf::Uint8)(a.b + (b.b - a.b) * t),
            (sf::Uint8)(a.a + (b.a - a.a) * t)
        );
    }

    void applyAppearance() {
        sf::Color boxCol = lerpColor(idleBoxColor, selBoxColor, transition);
        sf::Color txtCol = lerpColor(idleTextColor, selTextColor, transition);
        sf::Color outCol = lerpColor(idleOutlineColor, selOutlineColor, transition);

        box.setFillColor(boxCol);
        box.setOutlineColor(outCol);

        float thick = 2.0f + 3.0f * transition;
        box.setOutlineThickness(thick);

        text.setFillColor(txtCol);

        sf::FloatRect b = text.getLocalBounds();
        text.setPosition(
            box.getPosition().x + (box.getSize().x - b.width) / 2.0f - b.left,
            box.getPosition().y + (box.getSize().y - b.height) / 2.0f - b.top
        );
    }

public:
    MenuItem() {
        selected = false;
        transition = 0.0f;
    }

    void setup(const string& label, const sf::Font& font,
        float x, float y, float w, float h, const sf::Color& themeColor) {
        box.setSize(sf::Vector2f(w, h));
        box.setPosition(x, y);

        idleBoxColor = sf::Color(0, 0, 0, 140);
        selBoxColor = sf::Color(themeColor.r / 3, themeColor.g / 3, themeColor.b / 3, 220);

        idleOutlineColor = sf::Color(130, 130, 130);
        selOutlineColor = themeColor;

        idleTextColor = sf::Color(220, 220, 220);
        selTextColor = sf::Color::White;

        text.setFont(font);
        text.setString(label);
        text.setCharacterSize(32);

        selected = false;
        transition = 0.0f;

        applyAppearance();
    }

    void select() {
        selected = true;
    }

    void deselect() {
        selected = false;
    }

    void update(float dt) {
        float speed = 8.0f;
        float target = selected ? 1.0f : 0.0f;

        if (transition < target) {
            transition += speed * dt;
            if (transition > target) transition = target;
        }
        else if (transition > target) {
            transition -= speed * dt;
            if (transition < target) transition = target;
        }

        applyAppearance();
    }

    void draw(sf::RenderWindow& win) {
        win.draw(box);
        win.draw(text);
    }
};




/// purpose: capture the player's name via on-screen text entry before starting gameplay.
/// parameters: window is the shared render target; font is reused to avoid reloading resources.
/// return: returns the confirmed player name or a default fallback if the window closes.
string getPlayerName(sf::RenderWindow& window, const sf::Font& font) {
    sf::Text title("ENTER YOUR NAME", font, 70);
    title.setFillColor(sf::Color::Cyan);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
    title.setPosition(960.0f, 300.0f);

    sf::Text instruction("Press ENTER when done", font, 30);
    instruction.setFillColor(sf::Color(150, 150, 255));
    sf::FloatRect instrBounds = instruction.getLocalBounds();
    instruction.setOrigin(instrBounds.width / 2.0f, instrBounds.height / 2.0f);
    instruction.setPosition(960.0f, 380.0f);

    sf::Text nameDisplay("", font, 50);
    nameDisplay.setFillColor(sf::Color::White);

    string playerName = "";


    static bool starsInit = false;
    static sf::CircleShape stars[200];
    if (!starsInit) {
        srand((unsigned int)time(nullptr));
        for (int i = 0; i < 200; i++) {
            float r = 1.0f + (rand() % 3);
            stars[i].setRadius(r);
            stars[i].setPosition((float)(rand() % 1920), (float)(rand() % 1080));
            int b = 150 + rand() % 100;
            stars[i].setFillColor(sf::Color((sf::Uint8)b, (sf::Uint8)b, (sf::Uint8)b));
        }
        starsInit = true;
    }

    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) {
                window.close();
                return "Player";
            }
            if (e.type == sf::Event::TextEntered) {
                if (e.text.unicode == '\b' && playerName.length() > 0) {

                    playerName.pop_back();
                }
                else if (e.text.unicode == '\r' || e.text.unicode == '\n') {

                    if (playerName.length() > 0) {
                        return playerName;
                    }
                }
                else if (e.text.unicode < 128 && playerName.length() < 15) {

                    if ((e.text.unicode >= 'a' && e.text.unicode <= 'z') ||
                        (e.text.unicode >= 'A' && e.text.unicode <= 'Z') ||
                        (e.text.unicode >= '0' && e.text.unicode <= '9') ||
                        e.text.unicode == ' ') {
                        playerName += static_cast<char>(e.text.unicode);
                    }
                }
            }
        }

        nameDisplay.setString(playerName + "_");
        sf::FloatRect nameBounds = nameDisplay.getLocalBounds();
        nameDisplay.setOrigin(nameBounds.width / 2.0f, nameBounds.height / 2.0f);
        nameDisplay.setPosition(960.0f, 500.0f);

        window.clear(sf::Color(5, 5, 25));


        for (int i = 0; i < 200; i++) {
            window.draw(stars[i]);
        }

        window.draw(title);
        window.draw(instruction);
        window.draw(nameDisplay);
        window.display();
    }

    return "Player";
}





/// purpose: drives the main menu flow that precedes intro and level states.
/// parameters: uses shared font assets and window references to keep resource usage low.
/// return: exposes selection state for the game controller.
class Menu {
private:
    MenuItem items[5];
    string labels[5] = {
        "START GAME", "HIGH SCORE", "LEVELS", "EXIT", "CREDITS"
    };

    TypewriterText title;
    TypewriterText subtitle;

    sf::Font titleFont;
    sf::Font menuFont;

    int selected;
    bool titleDone;
    bool subtitleDone;
    bool showMenu;

public:
    Menu() {
        selected = 0;
        titleDone = false;
        subtitleDone = false;
        showMenu = false;
    }

    bool loadFonts(const string& titlePath, const string& menuPath) {
        if (!titleFont.loadFromFile(titlePath)) {
            cout << "Error loading title font";
            return false;
        }
        if (!menuFont.loadFromFile(menuPath)) {
            cout << "Error loading menu font";
            return false;
        }
        return true;
    }

    void initialize(float W, float H) {
        float scale = W / 1920.0f;

        int titleSize = (int)(120.0f * scale);
        if (titleSize < 90) titleSize = 90;

        title.setup("GALAXY WARS", titleFont, (unsigned int)titleSize, sf::Color::Cyan);
        title.setPosition(W * 0.5f, H * 0.18f);
        title.start();

        int subSize = (int)(40.0f * scale);
        if (subSize < 28) subSize = 28;

        subtitle.setup("The Ultimate Space Battle", titleFont, (unsigned int)subSize, sf::Color(150, 150, 255));
        subtitle.setPosition(W * 0.5f, H * 0.27f);
        subtitle.setActive(false);

        float boxWidth = 460.0f * scale;
        float boxHeight = 70.0f * scale;
        float spacing = 85.0f * scale;
        float startY = H * 0.45f;

        sf::Color themes[5] = {
            sf::Color(0, 200, 255),
            sf::Color(100, 255, 100),
            sf::Color(255, 215, 0),
            sf::Color(0, 220, 140),
            sf::Color(255, 80, 80)
        };

        for (int i = 0; i < 5; i++) {
            items[i].setup(labels[i], menuFont,
                (W - boxWidth) * 0.5f,
                startY + i * spacing,
                boxWidth,
                boxHeight,
                themes[i]);
        }

        selected = 0;
        items[0].select();
        titleDone = false;
        subtitleDone = false;
        showMenu = false;
    }

    void update(float dt) {
        if (!titleDone) {
            title.update(dt);
            if (title.isDone()) {
                titleDone = true;
                subtitle.setActive(true);
                subtitle.start();
            }
        }
        else if (!subtitleDone) {
            subtitle.update(dt);
            if (subtitle.isDone()) {
                subtitleDone = true;
                showMenu = true;
            }
        }

        for (int i = 0; i < 5; i++) {
            items[i].update(dt);
        }
    }

    void handleInput(sf::Keyboard::Key key) {
        if (!showMenu) return;

        if (key == sf::Keyboard::Up) {
            items[selected].deselect();
            selected = (selected + 4) % 5;
            items[selected].select();
        }
        else if (key == sf::Keyboard::Down) {
            items[selected].deselect();
            selected = (selected + 1) % 5;
            items[selected].select();
        }
    }

    int getSelected() const {
        if (!showMenu) return -1;
        return selected;
    }

    void draw(sf::RenderWindow& win) {
        drawStars(win);
        title.draw(win);
        if (titleDone) subtitle.draw(win);
        if (showMenu) {
            for (int i = 0; i < 5; i++) {
                items[i].draw(win);
            }
        }
    }

private:
    void drawStars(sf::RenderWindow& win) {
        static bool init = false;
        static sf::CircleShape stars[200];

        if (!init) {
            srand((unsigned int)time(nullptr));
            for (int i = 0; i < 200; i++) {
                float r = 1.0f + (rand() % 3);
                stars[i].setRadius(r);
                stars[i].setPosition((float)(rand() % 1920), (float)(rand() % 1080));
                int b = 150 + rand() % 100;
                stars[i].setFillColor(sf::Color((sf::Uint8)b, (sf::Uint8)b, (sf::Uint8)b));
            }
            init = true;
        }

        for (int i = 0; i < 200; i++) {
            win.draw(stars[i]);
        }
    }
};




class IntroSequence {
public:
    IntroSequence()
        : currentLine(0),
        currentChar(0),
        typingAccumulator(0.0f),
        fadeAlpha(0.0f),
        textAlpha(255.0f),
        phase(PhaseIdle),
        finished(false),
        active(false),
        hasSound(false),
        typingInterval(0.05f),
        shortPause(1.0f),
        longPause(2.5f),
        fadeSpeed(200.0f),
        textFadeSpeed(150.0f),
        pauseTimer(0.0f)
    {
        lines[0] = "The countdown to annihilation has begun.";
        lines[1] = "Every second lost is a city erased.";
        lines[2] = "Pilot -- the world needs you now";

        for (int i = 0; i < 3; i++) {
            displayed[i] = "";
        }
    }

    bool loadFont(const string& fontPath) {
        if (!font.loadFromFile(fontPath)) {
            cout << "Failed to load intro font";
            return false;
        }
        return true;
    }

    bool loadTypingSound(const string& soundPath) {
        if (!typingBuffer.loadFromFile(soundPath)) {
            cout << "Failed to load typing sound";
            return false;
        }
        typingSound.setBuffer(typingBuffer);
        typingSound.setLoop(true);
        hasSound = true;
        return true;
    }

    void start(float screenWidth, float screenHeight) {
        active = true;
        finished = false;
        phase = PhaseFadeIn;
        fadeAlpha = 0.0f;
        textAlpha = 255.0f;
        pauseTimer = 0.0f;
        typingAccumulator = 0.0f;
        currentLine = 0;
        currentChar = 0;
        clock.restart();

        overlay.setSize(sf::Vector2f(screenWidth, screenHeight));
        overlay.setFillColor(sf::Color(0, 0, 0, 0));

        float centerX = screenWidth * 0.5f;
        float baseY = screenHeight * 0.4f;
        float lineSpacing = 60.0f;

        for (int i = 0; i < 3; i++) {
            texts[i].setFont(font);
            texts[i].setCharacterSize(36);
            texts[i].setFillColor(sf::Color(255, 255, 255, 255));
            displayed[i] = "";
            texts[i].setString("");
            centerText(i, centerX, baseY + i * lineSpacing);
        }


        skipText.setFont(font);
        skipText.setString("Press any key to skip");
        skipText.setCharacterSize(24);
        skipText.setFillColor(sf::Color(200, 200, 200, 255));
        sf::FloatRect skipBounds = skipText.getLocalBounds();
        skipText.setOrigin(skipBounds.left + skipBounds.width / 2.0f, skipBounds.top + skipBounds.height / 2.0f);
        skipText.setPosition(centerX, screenHeight * 0.85f);
    }

    bool isFinished() const {
        return finished;
    }

    void skip() {

        finished = true;
        active = false;
        phase = PhaseDone;
        if (hasSound && typingSound.getStatus() == sf::Sound::Playing) {
            typingSound.stop();
        }
    }

    bool isActive() const {
        return active && !finished;
    }

    void update() {
        if (!active || finished) return;

        float dt = clock.restart().asSeconds();

        switch (phase) {
        case PhaseFadeIn:
            updateFadeIn(dt);
            break;
        case PhaseTyping:
            updateTyping(dt);
            break;
        case PhasePause:
            updatePause(dt);
            break;
        case PhaseFadeOutText:
            updateFadeOutText(dt);
            break;
        case PhaseDone:
        case PhaseIdle:
        default:
            break;
        }
    }

    void draw(sf::RenderWindow& window) {
        if (!active) return;

        window.draw(overlay);

        if (phase != PhaseFadeIn && phase != PhaseIdle && phase != PhaseDone) {
            for (int i = 0; i < 3; i++) {
                window.draw(texts[i]);
            }

            window.draw(skipText);
        }
    }

private:
    string lines[3];
    string displayed[3];
    sf::Text texts[3];
    sf::Text skipText;
    sf::Font font;

    int currentLine;
    int currentChar;

    float typingInterval;
    float typingAccumulator;

    float shortPause;
    float longPause;
    float pauseTimer;

    float fadeAlpha;
    float fadeSpeed;

    float textAlpha;
    float textFadeSpeed;

    sf::RectangleShape overlay;
    sf::Clock clock;

    sf::SoundBuffer typingBuffer;
    sf::Sound typingSound;
    bool hasSound;


    static const int PhaseIdle = 0;
    static const int PhaseFadeIn = 1;
    static const int PhaseTyping = 2;
    static const int PhasePause = 3;
    static const int PhaseFadeOutText = 4;
    static const int PhaseDone = 5;

    int phase;
    bool finished;
    bool active;

    void centerText(int index, float cx, float cy) {
        sf::FloatRect b = texts[index].getLocalBounds();
        texts[index].setOrigin(b.left + b.width * 0.5f, b.top + b.height * 0.5f);
        texts[index].setPosition(cx, cy);
    }

    void updateFadeIn(float dt) {
        fadeAlpha += fadeSpeed * dt;
        if (fadeAlpha >= 255.0f) {
            fadeAlpha = 255.0f;
            overlay.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)fadeAlpha));
            phase = PhaseTyping;
            startTypingLine(0);
        }
        else {
            overlay.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)fadeAlpha));
        }
    }

    void updateTyping(float dt) {
        typingAccumulator += dt;

        if (typingAccumulator >= typingInterval) {
            typingAccumulator = 0.0f;

            const string& fullLine = lines[currentLine];
            if (currentChar < (int)fullLine.size()) {
                if (hasSound && typingSound.getStatus() != sf::Sound::Playing) {
                    typingSound.play();
                }

                displayed[currentLine] += fullLine[currentChar];
                currentChar++;

                texts[currentLine].setString(displayed[currentLine]);
                centerText(currentLine,
                    texts[currentLine].getPosition().x,
                    texts[currentLine].getPosition().y);
            }
            else {
                if (hasSound && typingSound.getStatus() == sf::Sound::Playing) {
                    typingSound.stop();
                }

                if (currentLine == 0) {
                    pauseTimer = 0.0f;
                    phase = PhasePause;
                }
                else if (currentLine == 1) {
                    pauseTimer = 0.0f;
                    phase = PhasePause;
                }
                else {
                    phase = PhaseFadeOutText;
                }
            }
        }
    }

    void updatePause(float dt) {
        pauseTimer += dt;

        float targetPause = (currentLine == 0) ? shortPause : longPause;

        if (pauseTimer >= targetPause) {
            currentLine++;
            if (currentLine < 3) {
                startTypingLine(currentLine);
                phase = PhaseTyping;
            }
            else {
                phase = PhaseFadeOutText;
            }
        }
    }

    void updateFadeOutText(float dt) {
        if (hasSound && typingSound.getStatus() == sf::Sound::Playing) {
            typingSound.stop();
        }

        textAlpha -= textFadeSpeed * dt;
        if (textAlpha <= 0.0f) {
            textAlpha = 0.0f;
            for (int i = 0; i < 3; i++) {
                texts[i].setFillColor(sf::Color(255, 255, 255, 0));
            }
            phase = PhaseDone;
            finished = true;
            active = false;
        }
        else {
            for (int i = 0; i < 3; i++) {
                texts[i].setFillColor(sf::Color(255, 255, 255, (sf::Uint8)textAlpha));
            }
        }
    }

    void startTypingLine(int lineIndex) {
        currentLine = lineIndex;
        currentChar = 0;
        typingAccumulator = 0.0f;
        displayed[lineIndex] = "";
        texts[lineIndex].setString("");
    }
};




class Level2Transition {
public:
    Level2Transition()
        : currentChar(0),
        typingSpeed(0.03f),
        typingTimer(0.0f),
        fadeAlpha(0.0f),
        fadeSpeed(250.0f),
        pauseDuration(1.0f),
        pauseTimer(0.0f),
        phase(PhaseIdle),
        finished(false),
        active(false),
        hasSound(false),
        centerX(960.0f),
        centerY(540.0f)
    {
        message = "Good work! Get ready for the next round.";
        displayedText = "";
    }

    bool loadFont(const string& fontPath) {
        if (!font.loadFromFile(fontPath)) {
            cout << "Failed to load transition font";
            return false;
        }
        return true;
    }

    bool loadSound(const string& soundPath) {
        if (!soundBuffer.loadFromFile(soundPath)) {
            cout << "Failed to load transition sound";
            return false;
        }
        sound.setBuffer(soundBuffer);
        sound.setLoop(true);
        hasSound = true;
        return true;
    }

    void start(float screenWidth, float screenHeight) {
        active = true;
        finished = false;
        phase = PhaseFadeIn;
        fadeAlpha = 0.0f;
        currentChar = 0;
        displayedText = "";
        typingTimer = 0.0f;
        pauseTimer = 0.0f;
        clock.restart();

        overlay.setSize(sf::Vector2f(screenWidth, screenHeight));
        overlay.setFillColor(sf::Color(0, 0, 0, 0));

        messageText.setFont(font);
        messageText.setString("");
        messageText.setCharacterSize(48);
        messageText.setFillColor(sf::Color(255, 255, 255, 255));

        centerX = screenWidth * 0.5f;
        centerY = screenHeight * 0.5f;
    }

    bool isFinished() const {
        return finished;
    }

    void skip() {

        finished = true;
        active = false;
        phase = PhaseDone;
        if (hasSound && sound.getStatus() == sf::Sound::Playing) {
            sound.stop();
        }
    }

    bool isActive() const {
        return active && !finished;
    }

    void update() {
        if (!active || finished) return;

        float dt = clock.restart().asSeconds();

        switch (phase) {
        case PhaseFadeIn:
            updateFadeIn(dt);
            break;
        case PhaseTyping:
            updateTyping(dt);
            break;
        case PhasePause:
            updatePause(dt);
            break;
        case PhaseFadeOut:
            updateFadeOut(dt);
            break;
        case PhaseDone:
        case PhaseIdle:
        default:
            break;
        }
    }

    void draw(sf::RenderWindow& window) {
        if (!active) return;

        window.draw(overlay);

        if (phase != PhaseIdle && phase != PhaseDone) {
            window.draw(messageText);
        }
    }

private:
    string message;
    string displayedText;
    sf::Text messageText;
    sf::Font font;

    int currentChar;
    float typingSpeed;
    float typingTimer;

    float fadeAlpha;
    float fadeSpeed;

    float pauseDuration;
    float pauseTimer;

    float centerX;
    float centerY;

    sf::RectangleShape overlay;
    sf::Clock clock;

    sf::SoundBuffer soundBuffer;
    sf::Sound sound;
    bool hasSound;


    static const int PhaseIdle = 0;
    static const int PhaseFadeIn = 1;
    static const int PhaseTyping = 2;
    static const int PhasePause = 3;
    static const int PhaseFadeOut = 4;
    static const int PhaseDone = 5;

    int phase;
    bool finished;
    bool active;

    void updateFadeIn(float dt) {
        fadeAlpha += fadeSpeed * dt;

        if (fadeAlpha >= 255.0f) {
            fadeAlpha = 255.0f;
            overlay.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)fadeAlpha));
            phase = PhaseTyping;


            if (hasSound) {
                sound.play();
            }
        }
        else {
            overlay.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)fadeAlpha));
        }
    }

    void updateTyping(float dt) {
        typingTimer += dt;

        if (typingTimer >= typingSpeed) {
            typingTimer = 0.0f;

            if (currentChar < (int)message.length()) {
                displayedText += message[currentChar];
                currentChar++;

                messageText.setString(displayedText);


                sf::FloatRect bounds = messageText.getLocalBounds();
                messageText.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
                messageText.setPosition(centerX, centerY);
            }
            else {

                if (hasSound && sound.getStatus() == sf::Sound::Playing) {
                    sound.stop();
                }
                phase = PhasePause;
                pauseTimer = 0.0f;
            }
        }
    }

    void updatePause(float dt) {
        pauseTimer += dt;

        if (pauseTimer >= pauseDuration) {
            phase = PhaseFadeOut;
        }
    }

    void updateFadeOut(float dt) {
        fadeAlpha -= fadeSpeed * dt;

        if (fadeAlpha <= 0.0f) {
            fadeAlpha = 0.0f;
            overlay.setFillColor(sf::Color(0, 0, 0, 0));
            phase = PhaseDone;
            finished = true;
            active = false;
        }
        else {
            overlay.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)fadeAlpha));


            sf::Uint8 textAlpha = (sf::Uint8)fadeAlpha;
            messageText.setFillColor(sf::Color(255, 255, 255, textAlpha));
        }
    }
};




class VictoryStory {
public:
    VictoryStory()
        : currentLine(0),
        currentChar(0),
        typingAccumulator(0.0f),
        fadeAlpha(0.0f),
        textAlpha(255.0f),
        phase(PhaseIdle),
        finished(false),
        active(false),
        hasSound(false),
        typingInterval(0.05f),
        shortPause(1.0f),
        longPause(2.5f),
        fadeSpeed(200.0f),
        textFadeSpeed(150.0f),
        pauseTimer(0.0f)
    {
        lines[0] = "The threat has been eliminated.";
        lines[1] = "The galaxy is safe... for now.";
        lines[2] = "You are the hero we needed.";

        for (int i = 0; i < 3; i++) {
            displayed[i] = "";
        }
    }

    bool loadFont(const string& fontPath) {
        if (!font.loadFromFile(fontPath)) {
            cout << "Failed to load victory story font";
            return false;
        }
        return true;
    }

    bool loadTypingSound(const string& soundPath) {
        if (!typingBuffer.loadFromFile(soundPath)) {
            cout << "Failed to load victory typing sound";
            return false;
        }
        typingSound.setBuffer(typingBuffer);
        typingSound.setLoop(true);
        hasSound = true;
        return true;
    }

    void start(float screenWidth, float screenHeight) {
        active = true;
        finished = false;
        phase = PhaseFadeIn;
        fadeAlpha = 0.0f;
        textAlpha = 255.0f;
        pauseTimer = 0.0f;
        typingAccumulator = 0.0f;
        currentLine = 0;
        currentChar = 0;
        clock.restart();

        overlay.setSize(sf::Vector2f(screenWidth, screenHeight));
        overlay.setFillColor(sf::Color(0, 0, 0, 0));

        float centerX = screenWidth * 0.5f;
        float baseY = screenHeight * 0.4f;
        float lineSpacing = 60.0f;

        for (int i = 0; i < 3; i++) {
            texts[i].setFont(font);
            texts[i].setCharacterSize(36);
            texts[i].setFillColor(sf::Color(255, 255, 255, 255));
            displayed[i] = "";
            texts[i].setString("");
            centerText(i, centerX, baseY + i * lineSpacing);
        }

        skipText.setFont(font);
        skipText.setString("Press any key to skip");
        skipText.setCharacterSize(24);
        skipText.setFillColor(sf::Color(200, 200, 200, 255));
        sf::FloatRect skipBounds = skipText.getLocalBounds();
        skipText.setOrigin(skipBounds.left + skipBounds.width / 2.0f, skipBounds.top + skipBounds.height / 2.0f);
        skipText.setPosition(centerX, screenHeight * 0.85f);
    }

    bool isFinished() const {
        return finished;
    }

    void skip() {
        finished = true;
        active = false;
        phase = PhaseDone;
        if (hasSound && typingSound.getStatus() == sf::Sound::Playing) {
            typingSound.stop();
        }
    }

    bool isActive() const {
        return active && !finished;
    }

    void update() {
        if (!active || finished) return;

        float dt = clock.restart().asSeconds();

        switch (phase) {
        case PhaseFadeIn:
            updateFadeIn(dt);
            break;
        case PhaseTyping:
            updateTyping(dt);
            break;
        case PhasePause:
            updatePause(dt);
            break;
        case PhaseFadeOutText:
            updateFadeOutText(dt);
            break;
        case PhaseDone:
        case PhaseIdle:
        default:
            break;
        }
    }

    void draw(sf::RenderWindow& window) {
        if (!active) return;

        window.draw(overlay);

        if (phase != PhaseFadeIn && phase != PhaseIdle && phase != PhaseDone) {
            for (int i = 0; i < 3; i++) {
                window.draw(texts[i]);
            }
            window.draw(skipText);
        }
    }

private:
    string lines[3];
    string displayed[3];
    sf::Text texts[3];
    sf::Text skipText;
    sf::Font font;

    int currentLine;
    int currentChar;

    float typingInterval;
    float typingAccumulator;

    float shortPause;
    float longPause;
    float pauseTimer;

    float fadeAlpha;
    float fadeSpeed;

    float textAlpha;
    float textFadeSpeed;

    sf::RectangleShape overlay;
    sf::Clock clock;

    sf::SoundBuffer typingBuffer;
    sf::Sound typingSound;
    bool hasSound;

    static const int PhaseIdle = 0;
    static const int PhaseFadeIn = 1;
    static const int PhaseTyping = 2;
    static const int PhasePause = 3;
    static const int PhaseFadeOutText = 4;
    static const int PhaseDone = 5;

    int phase;
    bool finished;
    bool active;

    void centerText(int index, float cx, float cy) {
        sf::FloatRect b = texts[index].getLocalBounds();
        texts[index].setOrigin(b.left + b.width * 0.5f, b.top + b.height * 0.5f);
        texts[index].setPosition(cx, cy);
    }

    void updateFadeIn(float dt) {
        fadeAlpha += fadeSpeed * dt;
        if (fadeAlpha >= 255.0f) {
            fadeAlpha = 255.0f;
            overlay.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)fadeAlpha));
            phase = PhaseTyping;
            startTypingLine(0);
        }
        else {
            overlay.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)fadeAlpha));
        }
    }

    void updateTyping(float dt) {
        typingAccumulator += dt;

        if (typingAccumulator >= typingInterval) {
            typingAccumulator = 0.0f;

            const string& fullLine = lines[currentLine];
            if (currentChar < (int)fullLine.size()) {
                if (hasSound && typingSound.getStatus() != sf::Sound::Playing) {
                    typingSound.play();
                }

                displayed[currentLine] += fullLine[currentChar];
                currentChar++;

                texts[currentLine].setString(displayed[currentLine]);
                centerText(currentLine,
                    texts[currentLine].getPosition().x,
                    texts[currentLine].getPosition().y);
            }
            else {
                if (hasSound && typingSound.getStatus() == sf::Sound::Playing) {
                    typingSound.stop();
                }

                if (currentLine == 0) {
                    pauseTimer = 0.0f;
                    phase = PhasePause;
                }
                else if (currentLine == 1) {
                    pauseTimer = 0.0f;
                    phase = PhasePause;
                }
                else {
                    phase = PhaseFadeOutText;
                }
            }
        }
    }

    void updatePause(float dt) {
        pauseTimer += dt;

        float targetPause = (currentLine == 0) ? shortPause : longPause;

        if (pauseTimer >= targetPause) {
            currentLine++;
            if (currentLine < 3) {
                startTypingLine(currentLine);
                phase = PhaseTyping;
            }
            else {
                phase = PhaseFadeOutText;
            }
        }
    }

    void updateFadeOutText(float dt) {
        if (hasSound && typingSound.getStatus() == sf::Sound::Playing) {
            typingSound.stop();
        }

        textAlpha -= textFadeSpeed * dt;
        if (textAlpha <= 0.0f) {
            textAlpha = 0.0f;
            for (int i = 0; i < 3; i++) {
                texts[i].setFillColor(sf::Color(255, 255, 255, 0));
            }
            phase = PhaseDone;
            finished = true;
            active = false;
        }
        else {
            for (int i = 0; i < 3; i++) {
                texts[i].setFillColor(sf::Color(255, 255, 255, (sf::Uint8)textAlpha));
            }
        }
    }

    void startTypingLine(int lineIndex) {
        currentLine = lineIndex;
        currentChar = 0;
        typingAccumulator = 0.0f;
        displayed[lineIndex] = "";
        texts[lineIndex].setString("");
    }
};




class GameException : public exception {
private:
    string message;
    string errorType;

public:
    GameException(const string& msg, const string& type = "General")
        : message(msg), errorType(type) {
    }

    const char* what() const noexcept override {
        return message.c_str();
    }

    string getType() const {
        return errorType;
    }
};

class FileLoadException : public GameException {
public:
    FileLoadException(const string& filename)
        : GameException("Failed to load file: " + filename, "FileLoad") {
    }
};

class InvalidStateException : public GameException {
public:
    InvalidStateException(const string& msg)
        : GameException(msg, "InvalidState") {
    }
};





class Resource {
private:
    int value;

public:
    Resource(int v = 0) : value(v) {}


    Resource operator+(const Resource& other) const {
        return Resource(this->value + other.value);
    }

    int getValue() const { return value; }
    void setValue(int v) { value = v; }
};




template<typename T>
class ObjectPool {
private:
    T* objects;
    bool* activeStates;
    int poolSize;
    int activeCount;

public:
    ObjectPool(int size) : poolSize(size), activeCount(0) {
        objects = new T[poolSize];
        activeStates = new bool[poolSize];
        for (int i = 0; i < poolSize; i++) {
            activeStates[i] = false;
        }
    }

    ~ObjectPool() {
        delete[] objects;
        delete[] activeStates;
    }


    T* getInactive() {
        for (int i = 0; i < poolSize; i++) {
            if (!activeStates[i]) {
                activeStates[i] = true;
                activeCount++;
                return &objects[i];
            }
        }
        return nullptr;
    }


    void release(T* obj) {
        for (int i = 0; i < poolSize; i++) {
            if (&objects[i] == obj) {
                activeStates[i] = false;
                activeCount--;
                break;
            }
        }
    }


    T* get(int index) {
        if (index >= 0 && index < poolSize) {
            return &objects[index];
        }
        return nullptr;
    }


    bool isActive(int index) const {
        if (index >= 0 && index < poolSize) {
            return activeStates[index];
        }
        return false;
    }

    int getPoolSize() const { return poolSize; }
    int getActiveCount() const { return activeCount; }
    int getInactiveCount() const { return poolSize - activeCount; }


    void clear() {
        for (int i = 0; i < poolSize; i++) {
            activeStates[i] = false;
        }
        activeCount = 0;
    }
};




class PowerUp {
private:
    sf::Sprite sprite;
    sf::Texture textures[4];
    int type;
    bool active;
    float speed;
    int screenHeight;

public:
    PowerUp() : type(0), active(false), speed(200.0f), screenHeight(1080) {}

    bool loadTextures() {

        if (!textures[0].loadFromFile("powerupRed_bolt.png")) return false;
        if (!textures[1].loadFromFile("powerupGreen_shield.png")) return false;
        if (!textures[2].loadFromFile("pill_blue.png")) return false;
        if (!textures[3].loadFromFile("bolt_gold.png")) return false;
        return true;
    }

    void setScreenHeight(int height) {
        screenHeight = height;
    }

    void spawn(float x, int powerType) {
        active = true;
        type = powerType;
        sprite.setTexture(textures[type]);


        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);

        sprite.setPosition(x, -50.0f);
    }

    void update(float dt) {
        if (!active) return;
        sprite.move(0.0f, speed * dt);


        if (sprite.getPosition().y > screenHeight + 50) {
            active = false;
        }
    }

    void draw(sf::RenderWindow& window) {
        if (active) window.draw(sprite);
    }

    bool isActive() const { return active; }
    void deactivate() { active = false; }
    int getType() const { return type; }
    sf::FloatRect getBounds() const { return sprite.getGlobalBounds(); }
};




class EnemyBullet {
private:
    sf::Sprite sprite;
    sf::Texture texture;
    float speed;
    bool active;
    int screenHeight;

public:
    EnemyBullet() : speed(300.0f), active(false), screenHeight(1080) {}

    bool loadTexture(const string& texturePath) {
        if (texture.loadFromFile(texturePath)) {
            sprite.setTexture(texture);
            sf::FloatRect bounds = sprite.getLocalBounds();
            sprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
            return true;
        }
        return false;
    }

    void setScreenHeight(int height) {
        screenHeight = height;
    }

    void fire(float x, float y) {
        active = true;
        sprite.setPosition(x, y);
    }

    void update(float dt) {
        if (!active) return;
        sprite.move(0.0f, speed * dt);

        if (sprite.getPosition().y > screenHeight + 50) {
            active = false;
        }
    }

    void draw(sf::RenderWindow& window) {
        if (active) window.draw(sprite);
    }

    bool isActive() const { return active; }
    void deactivate() { active = false; }
    sf::FloatRect getBounds() const { return sprite.getGlobalBounds(); }
};







class Enemy {
protected:
    sf::Sprite sprite;
    sf::Texture texture;
    float speed;
    int health;
    bool active;
    int screenWidth;
    int screenHeight;


    static int totalEnemiesSpawned;
    static int totalEnemiesDestroyed;

public:
    Enemy() : speed(100.0f), health(1), active(false), screenWidth(1920), screenHeight(1080) {}

    virtual ~Enemy() {}

    void setScreenSize(int width, int height) {
        screenWidth = width;
        screenHeight = height;
    }

    virtual void spawn(float x, float y) {
        active = true;
        health = 1;
        sprite.setPosition(x, y);
        totalEnemiesSpawned++;
    }


    virtual void update(float dt) = 0;


    virtual void shoot(EnemyBullet* bullets, int maxBullets) {

        for (int i = 0; i < maxBullets; i++) {
            if (!bullets[i].isActive()) {
                sf::Vector2f pos = sprite.getPosition();
                bullets[i].fire(pos.x, pos.y + 20.0f);
                break;
            }
        }
    }

    void takeDamage() {
        health--;
        if (health <= 0) {
            active = false;
            totalEnemiesDestroyed++;
        }
    }

    void draw(sf::RenderWindow& window) {
        if (active) window.draw(sprite);
    }

    bool isActive() const { return active; }
    void deactivate() {
        if (active) {
            active = false;
            totalEnemiesDestroyed++;
        }
    }
    sf::FloatRect getBounds() const { return sprite.getGlobalBounds(); }


    static int getTotalSpawned() { return totalEnemiesSpawned; }
    static int getTotalDestroyed() { return totalEnemiesDestroyed; }
    static void resetStatistics() {
        totalEnemiesSpawned = 0;
        totalEnemiesDestroyed = 0;
    }


    friend void debugEnemyStats(const Enemy& enemy);
};


int Enemy::totalEnemiesSpawned = 0;
int Enemy::totalEnemiesDestroyed = 0;


void debugEnemyStats(const Enemy& enemy) {

    cout << "=== ENEMY DEBUG INFO ===" << endl;
    cout << "Health: " << enemy.health << endl;
    cout << "Speed: " << enemy.speed << endl;
    cout << "Active: " << (enemy.active ? "Yes" : "No") << endl;
    cout << "Position: (" << enemy.sprite.getPosition().x << ", "
        << enemy.sprite.getPosition().y << ")" << endl;
    cout << "Total Spawned: " << Enemy::getTotalSpawned() << endl;
    cout << "Total Destroyed: " << Enemy::getTotalDestroyed() << endl;
    cout << "========================" << endl;
}




class EnemyLevel1 : public Enemy {
private:
    sf::Clock shootTimer;
    float shootInterval;
    int movementPattern;
    float movementTimer;
    float initialX;

public:
    EnemyLevel1(const string& color = "Red", float enemySpeed = 80.0f, int pattern = 0) {
        speed = enemySpeed;
        health = 1;
        shootInterval = 3.0f;
        movementPattern = pattern;
        movementTimer = 0.0f;
        initialX = 0.0f;

        string filename = "enemy" + color + "1.png";
        if (texture.loadFromFile(filename)) {
            sprite.setTexture(texture);
            sf::FloatRect bounds = sprite.getLocalBounds();
            sprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
            sprite.setScale(0.8f, 0.8f);
        }
    }

    void spawn(float x, float y) {
        Enemy::spawn(x, y);
        shootTimer.restart();
        initialX = x;
        movementTimer = 0.0f;
    }


    void update(float dt) {
        if (!active) return;

        movementTimer += dt;

        if (movementPattern == 0) {

            sprite.move(0.0f, speed * dt);
        }
        else if (movementPattern == 1) {

            float horizontalSpeed = 150.0f;
            float frequency = 2.0f;


            sprite.move(0.0f, speed * dt);


            float horizontalOffset = sin(movementTimer * frequency) * horizontalSpeed * dt;
            sprite.move(horizontalOffset, 0.0f);
        }
        else if (movementPattern == 2) {

            float horizontalSpeed = 200.0f;
            float switchTime = 1.0f;


            sprite.move(0.0f, speed * dt);


            int segment = (int)(movementTimer / switchTime) % 4;

            if (segment == 0) {

                sprite.move(horizontalSpeed * dt, 0.0f);
            }
            else if (segment == 1) {

            }
            else if (segment == 2) {

                sprite.move(-horizontalSpeed * dt, 0.0f);
            }
            else if (segment == 3) {

            }
        }


        sf::Vector2f pos = sprite.getPosition();
        if (pos.y > screenHeight + 50 || pos.x < -100 || pos.x > screenWidth + 100) {
            active = false;
        }
    }


    bool shouldShoot() {
        if (shootTimer.getElapsedTime().asSeconds() >= shootInterval) {
            shootTimer.restart();
            return true;
        }
        return false;
    }
};





class BossEnemy : public Enemy {
private:
    sf::Clock shootTimer;
    float shootInterval;
    int maxHealth;
    sf::RectangleShape healthBarBg;
    sf::RectangleShape healthBarFill;

public:
    BossEnemy(const string& color = "Black", float bossSpeed = 50.0f) {
        speed = bossSpeed;
        health = 50;
        maxHealth = 50;
        shootInterval = 1.0f;

        string filename = "boss_blue.png";
        if (texture.loadFromFile(filename)) {
            sprite.setTexture(texture);
            sf::FloatRect bounds = sprite.getLocalBounds();
            sprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
            sprite.setScale(1.4f, 1.4f);
        }


        healthBarBg.setSize(sf::Vector2f(200.0f, 15.0f));
        healthBarBg.setFillColor(sf::Color(50, 50, 50, 200));
        healthBarBg.setOutlineThickness(2);
        healthBarBg.setOutlineColor(sf::Color::White);

        healthBarFill.setSize(sf::Vector2f(200.0f, 15.0f));
        healthBarFill.setFillColor(sf::Color::Red);
    }

    void spawn(float x, float y) {
        Enemy::spawn(x, y);
        health = maxHealth;
        shootTimer.restart();
    }


    void update(float dt) {
        if (!active) return;


        sprite.move(speed * dt, 0.0f);


        sf::Vector2f pos = sprite.getPosition();
        if (pos.x < 100.0f || pos.x > screenWidth - 100.0f) {
            speed = -speed;
        }


        updateHealthBar();
    }

    void updateHealthBar() {
        sf::Vector2f pos = sprite.getPosition();
        healthBarBg.setPosition(pos.x - 100.0f, pos.y - 120.0f);

        float healthPercent = (float)health / (float)maxHealth;
        healthBarFill.setSize(sf::Vector2f(200.0f * healthPercent, 15.0f));
        healthBarFill.setPosition(pos.x - 100.0f, pos.y - 120.0f);


        if (healthPercent > 0.6f) {
            healthBarFill.setFillColor(sf::Color::Green);
        }
        else if (healthPercent > 0.3f) {
            healthBarFill.setFillColor(sf::Color::Yellow);
        }
        else {
            healthBarFill.setFillColor(sf::Color::Red);
        }
    }


    bool shouldShoot() {
        if (shootTimer.getElapsedTime().asSeconds() >= shootInterval) {
            shootTimer.restart();
            return true;
        }
        return false;
    }


    void shoot(EnemyBullet* bullets, int maxBullets) {
        int bulletsShot = 0;
        for (int i = 0; i < maxBullets && bulletsShot < 3; i++) {
            if (!bullets[i].isActive()) {
                sf::Vector2f pos = sprite.getPosition();
                float offsetX = (bulletsShot - 1) * 40.0f;
                bullets[i].fire(pos.x + offsetX, pos.y + 50.0f);
                bulletsShot++;
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        if (active) {
            window.draw(sprite);

            window.draw(healthBarBg);
            window.draw(healthBarFill);
        }
    }
};




struct FormationPosition {
    float x;
    float y;
    bool filled;

    FormationPosition() : x(0), y(0), filled(false) {}
    FormationPosition(float px, float py) : x(px), y(py), filled(false) {}
};

class EnemyFormation {
public:
    static const int MAX_POSITIONS = 20;
    FormationPosition positions[MAX_POSITIONS];
    int positionCount;

    EnemyFormation() : positionCount(0) {}


    void createVFormation(float centerX, float startY, int enemyCount) {
        positionCount = 0;
        int perSide = enemyCount / 2;
        float spacing = 80.0f;

        for (int i = 0; i < perSide && positionCount < MAX_POSITIONS; i++) {

            positions[positionCount++] = FormationPosition(
                centerX - (i + 1) * spacing,
                startY + i * spacing
            );

            positions[positionCount++] = FormationPosition(
                centerX + (i + 1) * spacing,
                startY + i * spacing
            );
        }
    }


    void createTriangleFormation(float centerX, float startY, int enemyCount) {
        positionCount = 0;
        float spacing = 80.0f;
        int rows = 4;
        int enemiesPerRow = 1;

        for (int row = 0; row < rows && positionCount < enemyCount && positionCount < MAX_POSITIONS; row++) {
            for (int col = 0; col < enemiesPerRow && positionCount < enemyCount && positionCount < MAX_POSITIONS; col++) {
                float offsetX = (col - (enemiesPerRow - 1) / 2.0f) * spacing;
                positions[positionCount++] = FormationPosition(
                    centerX + offsetX,
                    startY + row * spacing
                );
            }
            enemiesPerRow++;
        }
    }


    bool getNextPosition(float& outX, float& outY) {
        for (int i = 0; i < positionCount; i++) {
            if (!positions[i].filled) {
                outX = positions[i].x;
                outY = positions[i].y;
                positions[i].filled = true;
                return true;
            }
        }
        return false;
    }

    void reset() {
        for (int i = 0; i < positionCount; i++) {
            positions[i].filled = false;
        }
    }
};




class Bullet {
private:
    sf::Sprite sprite;
    sf::Texture texture;
    float speed;
    bool active;
    int screenHeight;
    sf::Vector2f velocity;


    int* damageMultiplier;

public:
    Bullet() : speed(800.0f), active(false), screenHeight(1080), velocity(0.0f, -1.0f) {
        damageMultiplier = new int(1);
    }


    Bullet(const Bullet& other)
        : sprite(other.sprite),
        texture(other.texture),
        speed(other.speed),
        active(other.active),
        screenHeight(other.screenHeight),
        velocity(other.velocity) {

        damageMultiplier = new int(*other.damageMultiplier);
        cout << "Bullet copy constructor called (deep copy)" << endl;
    }


    Bullet& operator=(const Bullet& other) {
        if (this != &other) {
            sprite = other.sprite;
            texture = other.texture;
            speed = other.speed;
            active = other.active;
            screenHeight = other.screenHeight;
            velocity = other.velocity;


            delete damageMultiplier;
            damageMultiplier = new int(*other.damageMultiplier);
        }
        return *this;
    }


    ~Bullet() {
        delete damageMultiplier;
    }

    bool loadTexture(const string& texturePath) {
        try {
            if (!texture.loadFromFile(texturePath)) {
                throw FileLoadException(texturePath);
            }
            sprite.setTexture(texture);
            sf::FloatRect bounds = sprite.getLocalBounds();
            sprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
            return true;
        }
        catch (const FileLoadException& e) {
            cout << "ERROR: " << e.what() << endl;
            return false;
        }
    }

    void fire(sf::Vector2f startPosition, float rotationDegrees = 0.0f) {
        active = true;
        sprite.setPosition(startPosition);
        sprite.setRotation(rotationDegrees);

        float rotationRadians = rotationDegrees * 3.14159265f / 180.0f;
        velocity.x = std::sin(rotationRadians);
        velocity.y = -std::cos(rotationRadians);
    }

    void update(float dt) {
        if (!active) return;
        sprite.move(velocity.x * speed * dt, velocity.y * speed * dt);

        sf::Vector2f pos = sprite.getPosition();
        if (pos.y < -50.0f || pos.y > screenHeight + 50.0f || pos.x < -50.0f || pos.x > 1920 + 50.0f) {
            deactivate();
        }
    }

    void draw(sf::RenderWindow& window) {
        if (active) {
            window.draw(sprite);
        }
    }

    void deactivate() {
        active = false;
    }

    bool isActive() const {
        return active;
    }

    void setScreenHeight(int height) {
        screenHeight = height;
    }

    sf::FloatRect getBounds() const {
        if (active) {
            return sprite.getGlobalBounds();
        }
        return sf::FloatRect(0, 0, 0, 0);
    }


    int getDamageMultiplier() const {
        return *damageMultiplier;
    }

    void setDamageMultiplier(int mult) {
        *damageMultiplier = mult;
    }
};




class Meteor {
private:
    sf::Sprite sprite;
    sf::Texture bigTexture;
    sf::Texture smallTexture;
    sf::Texture explosionTexture;
    float speed;
    bool active;
    bool isExploding;
    int type;
    int screenWidth;
    int screenHeight;
    sf::Clock explosionTimer;

public:
    Meteor() : speed(0.0f), active(false), isExploding(false), type(0), screenWidth(1920), screenHeight(1080) {
    }

    bool loadTextures() {

        if (!bigTexture.loadFromFile("meteorBrown_big1.png")) {
            cout << "Error loading big meteor texture!" << endl;
            return false;
        }


        if (!smallTexture.loadFromFile("meteorBrown_small1.png")) {
            cout << "Error loading small meteor texture!" << endl;
            return false;
        }


        if (!explosionTexture.loadFromFile("playerShip2_damage1.png")) {
            cout << "Warning: Could not load explosion texture!" << endl;

        }

        return true;
    }

    void setScreenSize(int width, int height) {
        screenWidth = width;
        screenHeight = height;
    }

    void spawn(float spawnScreenWidth) {
        active = true;
        isExploding = false;


        float randomX = (float)(rand() % (int)spawnScreenWidth);
        sprite.setPosition(randomX, -100.0f);


        type = rand() % 2;

        if (type == 1) {

            sprite.setTexture(bigTexture);
            speed = 150.0f + (rand() % 50);
            sprite.setScale(1.0f, 1.0f);
        }
        else {

            sprite.setTexture(smallTexture);
            speed = 250.0f + (rand() % 100);
            sprite.setScale(1.0f, 1.0f);
        }


        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);


        sprite.setRotation((float)(rand() % 360));
    }

    void takeDamage() {
        if (isExploding) return;

        isExploding = true;
        sprite.setTexture(explosionTexture);
        explosionTimer.restart();


        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
    }

    void update(float dt) {
        if (!active) return;

        if (isExploding) {

            if (explosionTimer.getElapsedTime().asSeconds() > 0.2f) {
                active = false;
                isExploding = false;
            }

        }
        else {

            sprite.move(0.0f, speed * dt);


            sprite.rotate(30.0f * dt);


            if (sprite.getPosition().y > screenHeight + 100.0f) {
                active = false;
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        if (active) {
            window.draw(sprite);
        }
    }

    void deactivate() {
        active = false;
        isExploding = false;
    }

    bool isActive() const {
        return active;
    }

    bool getIsExploding() const {
        return isExploding;
    }

    int getType() const {
        return type;
    }

    sf::FloatRect getBounds() const {
        if (active && !isExploding) {
            return sprite.getGlobalBounds();
        }
        return sf::FloatRect(0, 0, 0, 0);
    }

    sf::Vector2f getPosition() const {
        return sprite.getPosition();
    }
};


class Level1 {
private:
    sf::Texture bgTexture;
    sf::Sprite bgSprite;

    sf::Texture playerTexture;
    sf::Sprite player;


    Bullet bullets[20];
    sf::SoundBuffer laserSoundBuffer;
    sf::Sound laserSound;
    bool soundLoaded;
    bool spaceWasPressed;


    Meteor meteors[20];
    sf::Clock meteorSpawnTimer;
    float meteorSpawnInterval;


    bool isDestroyed;
    sf::Texture playerDestroyedTexture;
    int lives;


    PowerUp powerups[10];
    sf::Clock powerUpSpawnTimer;
    float powerUpSpawnInterval;


    bool hasShield;
    float shieldTimer;
    bool hasDoubleFire;
    float doubleFireTimer;
    sf::Texture shieldTexture;
    sf::Sprite shieldSprite;


    sf::RectangleShape powerUpFlash;
    sf::Clock powerUpFlashTimer;
    sf::Color powerUpFlashColor;
    bool showPowerUpFlash;


    Enemy* enemies[10];
    EnemyBullet enemyBullets[50];
    sf::Clock enemySpawnTimer;
    float enemySpawnInterval;
    string enemyColor;
    ObjectPool<Bullet>* bulletPool;


    int score;
    sf::Texture numeralTextures[10];
    sf::Sprite scoreDigits[6];


    sf::Texture lifeIconTexture;
    sf::Sprite lifeIcon;
    sf::Texture xTexture;
    sf::Sprite xSprite;
    sf::Sprite lifeCountSprite;

    float speed;
    float tiltAngle;
    float tiltSpeed;

    int screenW;
    int screenH;




    int currentWave;
    int maxWaves;
    int enemiesPerWave;
    int enemiesSpawnedInWave;
    bool waveInProgress;
    float waveDelay;

    int activeEnemiesCount;
    bool allWaveEnemiesCleared;


    TypewriterText waveAnnouncement;
    sf::Font waveFont;
    bool showingWaveAnnouncement;
    sf::Clock waveAnnouncementTimer;
    float waveAnnouncementDuration;
    sf::Clock waveDelayTimer;




    sf::Clock gameTimer;
    float elapsedTime;
    bool timerRunning;
    sf::Texture timerNumeralTextures[10];
    sf::Sprite timerDigits[6];
    int scoreOffset;





    EnemyFormation currentFormation;
    BossEnemy* bossEnemy;
    bool isBossWave;

public:
    Level1() {
        scoreOffset = 0;
        speed = 450.0f;
        tiltAngle = 0.0f;
        tiltSpeed = 250.0f;

        screenW = 1920;
        screenH = 1080;

        soundLoaded = false;
        spaceWasPressed = false;
        isDestroyed = false;
        score = 0;
        lives = 3;
        meteorSpawnInterval = 1.5f;
        powerUpSpawnInterval = 8.0f;

        bossEnemy = nullptr;
        isBossWave = false;


        hasShield = false;
        shieldTimer = 0.0f;
        hasDoubleFire = false;
        doubleFireTimer = 0.0f;
        showPowerUpFlash = false;


        powerUpFlash.setSize(sf::Vector2f((float)screenW, (float)screenH));
        powerUpFlash.setFillColor(sf::Color(255, 255, 255, 0));


        if (shieldTexture.loadFromFile("shield3.png")) {
            shieldSprite.setTexture(shieldTexture);
            sf::FloatRect bounds = shieldSprite.getLocalBounds();
            shieldSprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
            shieldSprite.setScale(1.2f, 1.2f);
        }



        bulletPool = new ObjectPool<Bullet>(20);
        for (int i = 0; i < 20; i++) {
            bulletPool->get(i)->loadTexture("laserRed02.png");
            bulletPool->get(i)->setScreenHeight(screenH);
        }

        for (int i = 0; i < 20; i++) {
            meteors[i].loadTextures();
            meteors[i].setScreenSize(screenW, screenH);
        }


        for (int i = 0; i < 10; i++) {
            powerups[i].loadTextures();
            powerups[i].setScreenHeight(screenH);
        }


        enemySpawnInterval = 1.5f;
        enemyColor = "Red";
        for (int i = 0; i < 10; i++) {
            enemies[i] = 0;
        }


        for (int i = 0; i < 50; i++) {
            enemyBullets[i].loadTexture("laserRed02.png");
            enemyBullets[i].setScreenHeight(screenH);
        }



        if (laserSoundBuffer.loadFromFile("sfx_laser1.ogg")) {
            laserSound.setBuffer(laserSoundBuffer);
            soundLoaded = true;
        }


        for (int i = 0; i < 10; i++) {
            string filename = "numeral" + std::to_string(i) + ".png";
            if (!numeralTextures[i].loadFromFile(filename)) {
                cout << "Warning: Could not load " << filename << endl;
            }
        }


        if (!lifeIconTexture.loadFromFile("playerLife1_red.png")) {
            cout << "Warning: Could not load life icon!" << endl;
        }
        lifeIcon.setTexture(lifeIconTexture);
        lifeIcon.setScale(1.0f, 1.0f);
        lifeIcon.setPosition(20.0f, 20.0f);


        if (!xTexture.loadFromFile("numeralX.png")) {
            cout << "Warning: Could not load numeralX.png!" << endl;
        }
        xSprite.setTexture(xTexture);
        xSprite.setScale(1.0f, 1.0f);
        xSprite.setPosition(70.0f, 23.0f);


        lifeCountSprite.setScale(1.0f, 1.0f);
        lifeCountSprite.setPosition(110.0f, 23.0f);
        updateLivesDisplay();


        updateScoreDisplay();


        currentWave = 0;
        maxWaves = 2;
        enemiesPerWave = 0;
        enemiesSpawnedInWave = 0;
        waveInProgress = false;
        waveDelay = 5.0f;
        activeEnemiesCount = 0;
        allWaveEnemiesCleared = true;
        showingWaveAnnouncement = false;
        elapsedTime = 0.0f;
        timerRunning = false;


        for (int i = 0; i < 10; i++) {
            timerNumeralTextures[i] = numeralTextures[i];
        }
        waveAnnouncementDuration = 2.5f;


    }

    ~Level1() {

        for (int i = 0; i < 10; i++) {
            if (enemies[i] != 0) {
                delete enemies[i];
                enemies[i] = 0;
            }
        }
        if (bossEnemy != nullptr) {
            delete bossEnemy;
            bossEnemy = nullptr;
        }


        delete bulletPool;
    }

    bool loadWaveFont(const string& fontPath) {
        if (!waveFont.loadFromFile(fontPath)) {
            return false;
        }
        return true;
    }

    bool loadAssets(const string& backgroundFile = "bg2.jpg", const string& shipColor = "Red") {
        if (!bgTexture.loadFromFile(backgroundFile)) {
            cout << "Error loading background!";
            return false;
        }
        bgSprite.setTexture(bgTexture);


        bgSprite.setScale(
            (float)screenW / bgTexture.getSize().x,
            (float)screenH / bgTexture.getSize().y
        );
        bgSprite.setPosition(0.0f, 0.0f);


        sf::Vector2u texSize = bgTexture.getSize();


        float scaleX = (float)screenW / texSize.x;
        float scaleY = (float)screenH / texSize.y;


        float scale = (scaleX > scaleY) ? scaleX : scaleY;

        bgSprite.setScale(scale, scale);


        float offsetX = ((float)screenW - (texSize.x * scale)) / 2.0f;
        float offsetY = ((float)screenH - (texSize.y * scale)) / 2.0f;
        bgSprite.setPosition(offsetX, offsetY);


        string shipFile = "playerShip1_red.png";
        if (shipColor == "Blue") {
            shipFile = "playerShip1_blue.png";
        }
        else if (shipColor == "Green") {
            shipFile = "playerShip1_green.png";
        }
        else if (shipColor == "Orange") {
            shipFile = "playerShip1_orange.png";
        }

        if (!playerTexture.loadFromFile(shipFile)) {
            cout << "Error loading player jet!";
            return false;
        }
        player.setTexture(playerTexture);
        player.setOrigin(
            playerTexture.getSize().x / 2.0f,
            playerTexture.getSize().y / 2.0f
        );
        player.setPosition((float)screenW / 2.0f, (float)screenH - 250.0f);


        if (!playerDestroyedTexture.loadFromFile("playerShip1_damage3.png")) {
            cout << "Warning: Could not load player destroyed texture!" << endl;
        }

        return true;
    }
    void setScoreOffset(int offset) {
        scoreOffset = offset;
    }

    void reset() {

        isDestroyed = false;
        score = 0;
        lives = 3;
        tiltAngle = 0.0f;
        spaceWasPressed = false;


        player.setTexture(playerTexture);
        player.setPosition((float)screenW / 2.0f, (float)screenH - 250.0f);
        player.setRotation(0.0f);


        for (int i = 0; i < 20; i++) {
            bulletPool->get(i)->deactivate();
        }


        for (int i = 0; i < 20; i++) {
            meteors[i].deactivate();
        }


        for (int i = 0; i < 10; i++) {
            powerups[i].deactivate();
        }


        hasShield = false;
        shieldTimer = 0.0f;
        hasDoubleFire = false;
        doubleFireTimer = 0.0f;


        for (int i = 0; i < 10; i++) {
            if (enemies[i] != 0) {
                enemies[i]->deactivate();
            }
        }
        for (int i = 0; i < 50; i++) {
            enemyBullets[i].deactivate();
        }


        currentWave = 0;
        enemiesPerWave = 0;
        enemiesSpawnedInWave = 0;
        waveInProgress = false;
        waveDelayTimer.restart();

        activeEnemiesCount = 0;
        allWaveEnemiesCleared = true;
        showingWaveAnnouncement = false;
        meteorSpawnTimer.restart();

        if (bossEnemy != nullptr) {
            delete bossEnemy;
            bossEnemy = nullptr;
        }
        isBossWave = false;
        currentFormation.reset();


        updateScoreDisplay();
        updateLivesDisplay();
    }
    void startTimer() {
        if (!timerRunning) {
            timerRunning = true;
            gameTimer.restart();
            elapsedTime = 0.0f;
        }
    }

    void continueTimer(float previousTime) {

        timerRunning = true;
        elapsedTime = previousTime;

    }


    void configureDifficulty(float meteorInterval, float enemyInterval, float playerSpd, const string& enemyCol = "Red", int waves = 2) {
        meteorSpawnInterval = meteorInterval;
        enemySpawnInterval = enemyInterval;
        speed = playerSpd;
        enemyColor = enemyCol;
        maxWaves = waves;
        currentWave = 0;
        enemiesPerWave = 0;
        enemiesSpawnedInWave = 0;
        waveInProgress = false;
    }

    int calculateTargetScore() {
        int targetScore = 0;

        if (maxWaves == 2) {
            targetScore = 260;
        }
        else if (maxWaves == 3 && enemyColor != "Green") {
            targetScore = 480;
        }
        else if (maxWaves == 3 && enemyColor == "Green") {




            targetScore = 650;
        }

        return targetScore;
    }

    bool isBossDefeated() const {

        if (bossEnemy != nullptr && !bossEnemy->isActive() && isBossWave) {
            return true;
        }
        return false;
    }

    void update(float dt) {
        if (timerRunning && !isDestroyed) {

            elapsedTime += dt;
            updateTimerDisplay();
        }

        if (showingWaveAnnouncement) {
            waveAnnouncement.update(dt);


            if (waveAnnouncementTimer.getElapsedTime().asSeconds() >= waveAnnouncementDuration) {
                showingWaveAnnouncement = false;
                waveAnnouncement.setActive(false);
            }


            return;
        }


        if (isDestroyed) {

            updateBullets(dt);
            updateMeteors(dt);
            return;
        }

        float moveX = 0.0f;
        float moveY = 0.0f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  moveX -= speed * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) moveX += speed * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    moveY -= speed * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  moveY += speed * dt;

        player.move(moveX, moveY);

        sf::Vector2f pos = player.getPosition();
        if (pos.x < 40.0f) pos.x = 40.0f;
        if (pos.x > screenW - 40.0f) pos.x = (float)screenW - 40.0f;
        if (pos.y < 40.0f) pos.y = 40.0f;
        if (pos.y > screenH - 40.0f) pos.y = (float)screenH - 40.0f;
        player.setPosition(pos);

        if (moveX < 0.0f)
            tiltAngle -= tiltSpeed * dt;
        else if (moveX > 0.0f)
            tiltAngle += tiltSpeed * dt;
        else
            tiltAngle *= 0.9f;

        if (tiltAngle > 25.0f)  tiltAngle = 25.0f;
        if (tiltAngle < -25.0f) tiltAngle = -25.0f;

        player.setRotation(tiltAngle);


        handleShooting();


        updateBullets(dt);


        spawnMeteors();


        updateMeteors(dt);


        spawnPowerUps();


        updatePowerUps(dt);


        if (hasShield) {
            shieldTimer -= dt;
            if (shieldTimer <= 0.0f) {
                hasShield = false;
                shieldTimer = 0.0f;
            }
        }
        if (hasDoubleFire) {
            doubleFireTimer -= dt;
            if (doubleFireTimer <= 0.0f) {
                hasDoubleFire = false;
                doubleFireTimer = 0.0f;
            }
        }


        if (showPowerUpFlash) {
            float elapsed = powerUpFlashTimer.getElapsedTime().asSeconds();
            if (elapsed < 0.3f) {
                float alpha = 100.0f * (1.0f - elapsed / 0.3f);
                sf::Color flashColor = powerUpFlashColor;
                flashColor.a = (sf::Uint8)alpha;
                powerUpFlash.setFillColor(flashColor);
            }
            else {
                showPowerUpFlash = false;
                powerUpFlash.setFillColor(sf::Color(255, 255, 255, 0));
            }
        }


        checkBulletMeteorCollisions();
        if (!hasShield) {
            checkPlayerMeteorCollisions();
        }
        checkPowerUpCollisions();


        spawnEnemies();
        updateEnemies(dt);
        updateEnemyBullets(dt);
        checkBulletEnemyCollisions();
        checkPlayerEnemyBulletCollisions();
    }
    float getCurrentTime() const {
        return elapsedTime;
    }

    void handleShooting() {
        bool spacePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);


        if (spacePressed && !spaceWasPressed) {
            if (hasDoubleFire) {

                int bulletsFired = 0;
                for (int i = 0; i < 20 && bulletsFired < 2; i++) {
                    if (!bulletPool->get(i)->isActive()) {
                        float rotationDegrees = player.getRotation();
                        float rotationRadians = rotationDegrees * 3.14159265f / 180.0f;

                        sf::FloatRect playerBounds = player.getLocalBounds();
                        float noseOffset = playerBounds.height / 2.0f;


                        float sideOffset = (bulletsFired == 0) ? -15.0f : 15.0f;

                        sf::Vector2f playerPos = player.getPosition();
                        float noseX = playerPos.x + noseOffset * std::sin(rotationRadians) + sideOffset * std::cos(rotationRadians);
                        float noseY = playerPos.y - noseOffset * std::cos(rotationRadians) + sideOffset * std::sin(rotationRadians);

                        bulletPool->get(i)->fire(sf::Vector2f(noseX, noseY), rotationDegrees);
                        bulletsFired++;
                    }
                }

                if (soundLoaded) laserSound.play();
            }
            else {

                for (int i = 0; i < 20; i++) {
                    if (!bulletPool->get(i)->isActive()) {
                        float rotationDegrees = player.getRotation();
                        float rotationRadians = rotationDegrees * 3.14159265f / 180.0f;

                        sf::FloatRect playerBounds = player.getLocalBounds();
                        float noseOffset = playerBounds.height / 2.0f;

                        sf::Vector2f playerPos = player.getPosition();
                        float noseX = playerPos.x + noseOffset * std::sin(rotationRadians);
                        float noseY = playerPos.y - noseOffset * std::cos(rotationRadians);

                        bulletPool->get(i)->fire(sf::Vector2f(noseX, noseY), rotationDegrees);

                        if (soundLoaded) laserSound.play();
                        break;
                    }
                }
            }
        }

        spaceWasPressed = spacePressed;
    }

    void updateBullets(float dt) {
        for (int i = 0; i < 20; i++) {
            bulletPool->get(i)->update(dt);
        }
    }

    void spawnMeteors() {

        if (meteorSpawnTimer.getElapsedTime().asSeconds() >= meteorSpawnInterval) {

            for (int i = 0; i < 20; i++) {
                if (!meteors[i].isActive()) {
                    meteors[i].spawn((float)screenW);
                    meteorSpawnTimer.restart();
                    break;
                }
            }
        }
    }

    void updateMeteors(float dt) {
        for (int i = 0; i < 20; i++) {
            meteors[i].update(dt);
        }
    }

    void checkBulletMeteorCollisions() {

        for (int i = 0; i < 20; i++) {
            if (!bulletPool->get(i)->isActive()) continue;

            sf::FloatRect bulletBounds = bulletPool->get(i)->getBounds();

            for (int j = 0; j < 20; j++) {
                if (!meteors[j].isActive() || meteors[j].getIsExploding()) continue;

                sf::FloatRect meteorBounds = meteors[j].getBounds();


                if (bulletBounds.intersects(meteorBounds)) {

                    bulletPool->get(i)->deactivate();
                    meteors[j].takeDamage();


                    if (meteors[j].getType() == 1) {
                        score += 20;
                    }
                    else {
                        score += 10;
                    }
                    updateScoreDisplay();

                    break;
                }
            }
        }
    }

    void checkPlayerMeteorCollisions() {
        if (isDestroyed) return;

        sf::FloatRect playerBounds = player.getGlobalBounds();

        for (int i = 0; i < 20; i++) {
            if (!meteors[i].isActive() || meteors[i].getIsExploding()) continue;

            sf::FloatRect meteorBounds = meteors[i].getBounds();


            if (playerBounds.intersects(meteorBounds)) {

                loseLife();
                meteors[i].takeDamage();
                break;
            }
        }
    }

    void loseLife() {
        lives--;
        updateLivesDisplay();

        if (lives <= 0) {

            isDestroyed = true;
            timerRunning = false;
            player.setTexture(playerDestroyedTexture);
            cout << "GAME OVER! Final Score: " << score << endl;
            cout << "Press R to restart or ESC for main menu" << endl;
        }
        else {

            player.setPosition((float)screenW / 2.0f, (float)screenH - 250.0f);
            player.setRotation(0.0f);
            tiltAngle = 0.0f;
            cout << "Lives remaining: " << lives << endl;
        }
    }
    void stopTimer() {
        timerRunning = false;
    }

    float getElapsedTime() const {
        return elapsedTime;
    }

    void spawnPowerUps() {
        if (powerUpSpawnTimer.getElapsedTime().asSeconds() >= powerUpSpawnInterval) {
            for (int i = 0; i < 10; i++) {
                if (!powerups[i].isActive()) {
                    float x = (float)(rand() % screenW);
                    int type = rand() % 4;
                    powerups[i].spawn(x, type);
                    powerUpSpawnTimer.restart();
                    break;
                }
            }
        }
    }

    void updatePowerUps(float dt) {
        for (int i = 0; i < 10; i++) {
            powerups[i].update(dt);
        }
    }

    void checkPowerUpCollisions() {
        sf::FloatRect playerBounds = player.getGlobalBounds();

        for (int i = 0; i < 10; i++) {
            if (!powerups[i].isActive()) continue;

            if (playerBounds.intersects(powerups[i].getBounds())) {
                int type = powerups[i].getType();

                switch (type) {
                case 0:
                    hasDoubleFire = true;
                    doubleFireTimer = 10.0f;
                    cout << "Double Fire activated!" << endl;
                    powerUpFlashColor = sf::Color(255, 150, 50, 100);
                    showPowerUpFlash = true;
                    powerUpFlashTimer.restart();
                    break;

                case 1:
                    hasShield = true;
                    shieldTimer = 10.0f;
                    cout << "Shield activated!" << endl;
                    powerUpFlashColor = sf::Color(50, 255, 100, 100);
                    showPowerUpFlash = true;
                    powerUpFlashTimer.restart();
                    break;

                case 2:
                    if (lives < 3) {
                        lives++;
                        updateLivesDisplay();
                        cout << "Life restored! Lives: " << lives << endl;
                        powerUpFlashColor = sf::Color(100, 150, 255, 100);
                        showPowerUpFlash = true;
                        powerUpFlashTimer.restart();
                    }
                    break;

                case 3:
                    score += 50;
                    updateScoreDisplay();
                    cout << "Score boost! +50 points. Total: " << score << endl;
                    powerUpFlashColor = sf::Color(255, 215, 0, 100);
                    showPowerUpFlash = true;
                    powerUpFlashTimer.restart();
                    break;
                }

                powerups[i].deactivate();
            }
        }
    }

    void spawnEnemies() {

        if (currentWave == 0 && !waveInProgress) {
            startNewWave();
        }


        if (waveInProgress && enemiesSpawnedInWave >= enemiesPerWave) {

            countActiveEnemies();


            if (activeEnemiesCount == 0 && !allWaveEnemiesCleared) {
                allWaveEnemiesCleared = true;
                waveInProgress = false;
                waveDelayTimer.restart();
                cout << "Wave " << currentWave << " cleared! Waiting for next wave..." << endl;
            }
        }


        if (!waveInProgress && allWaveEnemiesCleared && currentWave < maxWaves) {
            if (waveDelayTimer.getElapsedTime().asSeconds() >= waveDelay) {
                startNewWave();
            }
            return;
        }


        if (currentWave >= maxWaves && enemiesSpawnedInWave >= enemiesPerWave) {
            return;
        }


        if (enemiesSpawnedInWave >= enemiesPerWave) {
            return;
        }



        if (enemySpawnTimer.getElapsedTime().asSeconds() >= enemySpawnInterval) {
            if (isBossWave) {

                if (bossEnemy == nullptr && enemiesSpawnedInWave == 0) {
                    bossEnemy = new BossEnemy("Black", 50.0f);
                    bossEnemy->setScreenSize(screenW, screenH);
                    bossEnemy->spawn(screenW * 0.5f, 100.0f);
                    enemiesSpawnedInWave++;
                    enemySpawnTimer.restart();
                    cout << "BOSS SPAWNED!" << endl;
                }
            }
            else {

                for (int i = 0; i < 10; i++) {
                    if (enemies[i] == 0 || !enemies[i]->isActive()) {
                        if (enemies[i] != 0) {
                            delete enemies[i];
                        }

                        float enemySpeed = (maxWaves == 3) ? 120.0f : 80.0f;


                        int movementPattern = 0;
                        if (maxWaves == 3 && enemyColor == "Green") {
                            if (currentWave == 1) {
                                movementPattern = 1;
                            }
                            else if (currentWave == 2) {
                                movementPattern = 2;
                            }
                        }

                        enemies[i] = new EnemyLevel1(enemyColor, enemySpeed, movementPattern);
                        enemies[i]->setScreenSize(screenW, screenH);


                        float spawnX = 0.0f, spawnY = -50.0f;
                        if (maxWaves == 3 && enemyColor == "Green" && currentWave <= 2) {

                            if (currentFormation.getNextPosition(spawnX, spawnY)) {
                                enemies[i]->spawn(spawnX, spawnY);
                            }
                            else {

                                float randomX = (float)(rand() % screenW);
                                enemies[i]->spawn(randomX, -50.0f);
                            }
                        }
                        else {

                            float randomX = (float)(rand() % screenW);
                            enemies[i]->spawn(randomX, -50.0f);
                        }

                        enemiesSpawnedInWave++;
                        enemySpawnTimer.restart();
                        break;
                    }
                }
            }
        }
    }

    void countActiveEnemies() {
        activeEnemiesCount = 0;


        if (isBossWave && bossEnemy != nullptr && bossEnemy->isActive()) {
            activeEnemiesCount++;
        }


        for (int i = 0; i < 10; i++) {
            if (enemies[i] != 0 && enemies[i]->isActive()) {
                activeEnemiesCount++;
            }
        }
    }

    void startNewWave() {
        currentWave++;
        waveInProgress = true;
        enemiesSpawnedInWave = 0;
        allWaveEnemiesCleared = false;


        if (maxWaves == 3 && enemyColor == "Green" && currentWave == 3) {

            isBossWave = true;
            enemiesPerWave = 1;
            cout << "BOSS WAVE! Prepare for battle!" << endl;
        }
        else {
            isBossWave = false;


            if (maxWaves == 2) {
                if (currentWave == 1) {
                    enemiesPerWave = 5 + (rand() % 2);
                }
                else if (currentWave == 2) {
                    enemiesPerWave = 7 + (rand() % 2);
                }
            }
            else if (maxWaves == 3 && enemyColor != "Green") {
                if (currentWave == 1) {
                    enemiesPerWave = 5 + (rand() % 2);
                }
                else if (currentWave == 2) {
                    enemiesPerWave = 7 + (rand() % 2);
                }
                else if (currentWave == 3) {
                    enemiesPerWave = 10 + (rand() % 3);
                }
            }
            else if (maxWaves == 3 && enemyColor == "Green") {
                if (currentWave == 1) {
                    enemiesPerWave = 6 + (rand() % 2);
                    currentFormation.createVFormation(screenW * 0.5f, -100.0f, enemiesPerWave);
                }
                else if (currentWave == 2) {
                    enemiesPerWave = 8 + (rand() % 2);
                    currentFormation.createTriangleFormation(screenW * 0.5f, -100.0f, enemiesPerWave);
                }
            }
        }

        cout << "Wave " << currentWave << " started! Enemies: " << enemiesPerWave << endl;


        showWaveAnnouncement();

        enemySpawnTimer.restart();
    }

    void showWaveAnnouncement() {
        string message;
        if (isBossWave) {
            message = "BOSS INCOMING!";
            waveAnnouncement.setup(message, waveFont, 64, sf::Color::Red);
        }
        else {
            message = "WAVE " + std::to_string(currentWave) + " INCOMING!";
            waveAnnouncement.setup(message, waveFont, 64, sf::Color::Yellow);
        }
        waveAnnouncement.setPosition((float)screenW * 0.5f, (float)screenH * 0.5f);
        waveAnnouncement.start();
        showingWaveAnnouncement = true;
        waveAnnouncementTimer.restart();
    }


    void updateEnemies(float dt) {

        if (bossEnemy != nullptr && bossEnemy->isActive()) {
            bossEnemy->update(dt);


            BossEnemy* boss = dynamic_cast<BossEnemy*>(bossEnemy);
            if (boss && boss->shouldShoot()) {
                boss->shoot(enemyBullets, 50);
            }
        }


        for (int i = 0; i < 10; i++) {
            if (enemies[i] != 0 && enemies[i]->isActive()) {
                enemies[i]->update(dt);

                EnemyLevel1* enemy1 = dynamic_cast<EnemyLevel1*>(enemies[i]);
                if (enemy1 && enemy1->shouldShoot()) {
                    enemy1->shoot(enemyBullets, 50);
                }
            }
        }
    }

    void updateEnemyBullets(float dt) {
        for (int i = 0; i < 50; i++) {
            enemyBullets[i].update(dt);
        }
    }

    void checkBulletEnemyCollisions() {

        if (bossEnemy != nullptr && bossEnemy->isActive()) {
            for (int i = 0; i < 20; i++) {
                if (!bulletPool->get(i)->isActive()) continue;

                if (bulletPool->get(i)->getBounds().intersects(bossEnemy->getBounds())) {
                    bossEnemy->takeDamage();
                    bulletPool->get(i)->deactivate();

                    score += 10;
                    updateScoreDisplay();

                    if (!bossEnemy->isActive()) {
                        cout << "BOSS DEFEATED!" << endl;
                    }
                    break;
                }
            }
        }


        for (int i = 0; i < 20; i++) {
            if (!bulletPool->get(i)->isActive()) continue;

            for (int j = 0; j < 10; j++) {
                if (enemies[j] == 0 || !enemies[j]->isActive()) continue;

                if (bulletPool->get(i)->getBounds().intersects(enemies[j]->getBounds())) {
                    enemies[j]->takeDamage();
                    bulletPool->get(i)->deactivate();

                    score += 10;
                    updateScoreDisplay();
                    break;
                }
            }
        }
    }

    void checkPlayerEnemyBulletCollisions() {
        if (hasShield) return;

        sf::FloatRect playerBounds = player.getGlobalBounds();

        for (int i = 0; i < 50; i++) {
            if (!enemyBullets[i].isActive()) continue;

            if (playerBounds.intersects(enemyBullets[i].getBounds())) {
                loseLife();
                enemyBullets[i].deactivate();
                break;
            }
        }
    }

    void updateLivesDisplay() {
        if (lives >= 0 && lives <= 9) {
            lifeCountSprite.setTexture(numeralTextures[lives]);
        }
    }

    void updateScoreDisplay() {

        int displayScore = score + scoreOffset;
        string scoreStr = std::to_string(displayScore);


        for (int i = 0; i < 6; i++) {
            scoreDigits[i].setScale(0.0f, 0.0f);
        }


        int numDigits = (int)scoreStr.length();
        if (numDigits > 6) numDigits = 6;

        float digitSpacing = 50.0f;
        float totalWidth = (numDigits - 1) * digitSpacing;


        float startX = (float)screenW - 60.0f - totalWidth;


        for (int i = 0; i < numDigits; i++) {
            int digit = scoreStr[i] - '0';

            scoreDigits[i].setTexture(numeralTextures[digit]);
            scoreDigits[i].setScale(1.8f, 1.8f);
            scoreDigits[i].setPosition(startX + i * digitSpacing, 20.0f);
        }
    }

    void updateTimerDisplay() {

        int totalSeconds = (int)elapsedTime;
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;


        if (minutes > 99) minutes = 99;


        float centerX = (float)screenW * 0.5f;
        float startY = 20.0f;
        float digitSpacing = 40.0f;


        int minute1 = minutes / 10;
        int minute2 = minutes % 10;


        int second1 = seconds / 10;
        int second2 = seconds % 10;



        timerDigits[0].setTexture(timerNumeralTextures[minute1]);
        timerDigits[0].setScale(1.5f, 1.5f);
        timerDigits[0].setPosition(centerX - 2.5f * digitSpacing, startY);


        timerDigits[1].setTexture(timerNumeralTextures[minute2]);
        timerDigits[1].setScale(1.5f, 1.5f);
        timerDigits[1].setPosition(centerX - 1.5f * digitSpacing, startY);


        timerDigits[2].setTexture(timerNumeralTextures[second1]);
        timerDigits[2].setScale(1.5f, 1.5f);
        timerDigits[2].setPosition(centerX + 0.5f * digitSpacing, startY);


        timerDigits[3].setTexture(timerNumeralTextures[second2]);
        timerDigits[3].setScale(1.5f, 1.5f);
        timerDigits[3].setPosition(centerX + 1.5f * digitSpacing, startY);
    }

    bool isPlayerDestroyed() const {
        return isDestroyed;
    }

    int getScore() const {
        return score;
    }

    void drawBullets(sf::RenderWindow& window) {
        for (int i = 0; i < 20; i++) {
            bulletPool->get(i)->draw(window);
        }
    }

    void drawMeteors(sf::RenderWindow& window) {
        for (int i = 0; i < 20; i++) {
            meteors[i].draw(window);
        }
    }

    void drawPowerUps(sf::RenderWindow& window) {
        for (int i = 0; i < 10; i++) {
            powerups[i].draw(window);
        }
    }

    void drawEnemies(sf::RenderWindow& window) {

        if (bossEnemy != nullptr && bossEnemy->isActive()) {
            bossEnemy->draw(window);
        }


        for (int i = 0; i < 10; i++) {
            if (enemies[i] != 0) {
                enemies[i]->draw(window);
            }
        }
    }

    void drawEnemyBullets(sf::RenderWindow& window) {
        for (int i = 0; i < 50; i++) {
            enemyBullets[i].draw(window);
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(bgSprite);
        drawMeteors(window);
        drawPowerUps(window);
        drawEnemies(window);
        drawEnemyBullets(window);


        if (hasShield) {
            shieldSprite.setPosition(player.getPosition());
            shieldSprite.setRotation(player.getRotation());
            window.draw(shieldSprite);
        }

        window.draw(player);
        drawBullets(window);


        window.draw(lifeIcon);
        window.draw(xSprite);
        window.draw(lifeCountSprite);


        int displayScore = score + scoreOffset;
        string scoreStr = std::to_string(displayScore);
        for (int i = 0; i < (int)scoreStr.length() && i < 6; i++) {
            window.draw(scoreDigits[i]);
        }


        if (showPowerUpFlash) {
            window.draw(powerUpFlash);
        }


        float barWidth = 200.0f;
        float barHeight = 15.0f;
        float barX = (float)screenW / 2.0f - barWidth / 2.0f;
        float barY = 60.0f;

        if (hasDoubleFire && doubleFireTimer > 0.0f) {

            sf::RectangleShape barBg(sf::Vector2f(barWidth, barHeight));
            barBg.setPosition(barX, barY);
            barBg.setFillColor(sf::Color(50, 50, 50, 200));
            barBg.setOutlineThickness(2);
            barBg.setOutlineColor(sf::Color::White);
            window.draw(barBg);


            float fillWidth = (doubleFireTimer / 10.0f) * barWidth;
            sf::RectangleShape barFill(sf::Vector2f(fillWidth, barHeight));
            barFill.setPosition(barX, barY);


            if (doubleFireTimer < 5.0f) {
                barFill.setFillColor(sf::Color(255, 50, 50));
            }
            else {
                barFill.setFillColor(sf::Color(255, 100, 50));
            }
            window.draw(barFill);

            barY += 20.0f;
        }

        if (hasShield && shieldTimer > 0.0f) {

            sf::RectangleShape barBg(sf::Vector2f(barWidth, barHeight));
            barBg.setPosition(barX, barY);
            barBg.setFillColor(sf::Color(50, 50, 50, 200));
            barBg.setOutlineThickness(2);
            barBg.setOutlineColor(sf::Color::White);
            window.draw(barBg);


            float fillWidth = (shieldTimer / 10.0f) * barWidth;
            sf::RectangleShape barFill(sf::Vector2f(fillWidth, barHeight));
            barFill.setPosition(barX, barY);


            if (shieldTimer < 5.0f) {
                barFill.setFillColor(sf::Color(255, 50, 50));
            }
            else {
                barFill.setFillColor(sf::Color(50, 255, 100));
            }
            window.draw(barFill);
        }


        if (timerRunning || isDestroyed) {

            for (int i = 0; i < 4; i++) {
                window.draw(timerDigits[i]);
            }


            float centerX = (float)screenW * 0.5f;
            float colonY = 30.0f;


            sf::CircleShape colonDot1(4.0f);
            colonDot1.setFillColor(sf::Color::White);
            colonDot1.setPosition(centerX - 15.0f, colonY - 5.0f);
            window.draw(colonDot1);

            sf::CircleShape colonDot2(4.0f);
            colonDot2.setFillColor(sf::Color::White);
            colonDot2.setPosition(centerX - 15.0f, colonY + 15.0f);
            window.draw(colonDot2);
        }


        if (showingWaveAnnouncement) {
            waveAnnouncement.draw(window);
        }


    }
};


const int PAUSE_NOTHING = 0;
const int PAUSE_RESUME = 1;
const int PAUSE_RESTART = 2;
const int PAUSE_MAIN_MENU = 3;
const int PAUSE_EXIT = 4;




class PauseMenu {
private:


    sf::Text options[4];
    sf::RectangleShape optionBoxes[4];


    string labels[4] = { "Resume", "Restart", "Main Menu", "Exit Game" };


    sf::RectangleShape backgroundOverlay;
    sf::Font font;


    sf::Color idleColor;
    sf::Color hoverColor;
    sf::Color orangeColor;


    int hoveredIndex;
    float windowWidth;
    float windowHeight;
    bool fontLoaded;
    bool mouseWasPressed;


    void initializeOptions() {
        float centerX = windowWidth * 0.5f;
        float startY = windowHeight * 0.4f;
        float spacing = 90.0f;
        float boxWidth = 400.0f;
        float boxHeight = 70.0f;

        for (int i = 0; i < 4; i++) {

            options[i].setFont(font);
            options[i].setString(labels[i]);
            options[i].setCharacterSize(36);
            options[i].setFillColor(idleColor);


            sf::FloatRect textBounds = options[i].getLocalBounds();
            float textX = centerX - textBounds.width / 2.0f - textBounds.left;
            float textY = startY + i * spacing + (boxHeight - textBounds.height) / 2.0f - textBounds.top;
            options[i].setPosition(textX, textY);


            optionBoxes[i].setSize(sf::Vector2f(boxWidth, boxHeight));
            optionBoxes[i].setPosition(centerX - boxWidth / 2.0f, startY + i * spacing);
            optionBoxes[i].setFillColor(sf::Color(0, 0, 0, 150));
            optionBoxes[i].setOutlineThickness(2);
            optionBoxes[i].setOutlineColor(sf::Color(100, 100, 100));
        }
    }

    void updateHover(sf::Vector2f mousePos) {
        hoveredIndex = -1;

        for (int i = 0; i < 4; i++) {
            sf::FloatRect boxBounds = optionBoxes[i].getGlobalBounds();

            if (boxBounds.contains(mousePos)) {
                hoveredIndex = i;
                options[i].setFillColor(hoverColor);
                options[i].setCharacterSize(40);
                optionBoxes[i].setOutlineColor(orangeColor);
                optionBoxes[i].setOutlineThickness(3);


                sf::FloatRect textBounds = options[i].getLocalBounds();
                float centerX = windowWidth * 0.5f;
                float startY = windowHeight * 0.4f;
                float spacing = 90.0f;
                float boxHeight = 70.0f;
                float textX = centerX - textBounds.width / 2.0f - textBounds.left;
                float textY = startY + i * spacing + (boxHeight - textBounds.height) / 2.0f - textBounds.top;
                options[i].setPosition(textX, textY);
            }
            else {
                options[i].setFillColor(idleColor);
                options[i].setCharacterSize(36);
                optionBoxes[i].setOutlineColor(sf::Color(100, 100, 100));
                optionBoxes[i].setOutlineThickness(2);


                sf::FloatRect textBounds = options[i].getLocalBounds();
                float centerX = windowWidth * 0.5f;
                float startY = windowHeight * 0.4f;
                float spacing = 90.0f;
                float boxHeight = 70.0f;
                float textX = centerX - textBounds.width / 2.0f - textBounds.left;
                float textY = startY + i * spacing + (boxHeight - textBounds.height) / 2.0f - textBounds.top;
                options[i].setPosition(textX, textY);
            }
        }
    }

public:
    PauseMenu() : hoveredIndex(-1), fontLoaded(false), windowWidth(1920.0f), windowHeight(1080.0f), mouseWasPressed(false) {

        idleColor = sf::Color(255, 200, 150);
        hoverColor = sf::Color::White;
        orangeColor = sf::Color(255, 100, 50);


        backgroundOverlay.setSize(sf::Vector2f(windowWidth, windowHeight));
        backgroundOverlay.setFillColor(sf::Color(0, 0, 0, 180));
    }

    bool loadFont(const string& fontPath) {
        if (font.loadFromFile(fontPath)) {
            fontLoaded = true;
            if (windowWidth > 0 && windowHeight > 0) {
                initializeOptions();
            }
            return true;
        }

        if (font.loadFromFile("arial.ttf")) {
            fontLoaded = true;
            if (windowWidth > 0 && windowHeight > 0) {
                initializeOptions();
            }
            return true;
        }
        return false;
    }

    void initialize(float width, float height) {
        windowWidth = width;
        windowHeight = height;
        backgroundOverlay.setSize(sf::Vector2f(width, height));

        if (fontLoaded) {
            initializeOptions();
        }
    }

    int handleInput(sf::RenderWindow& window) {

        sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePos = window.mapPixelToCoords(mousePixelPos);


        updateHover(mousePos);


        bool mouseIsPressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);

        if (mouseIsPressed && !mouseWasPressed) {

            if (hoveredIndex >= 0 && hoveredIndex < 4) {
                mouseWasPressed = true;

                if (hoveredIndex == 0) return PAUSE_RESUME;
                if (hoveredIndex == 1) return PAUSE_RESTART;
                if (hoveredIndex == 2) return PAUSE_MAIN_MENU;
                if (hoveredIndex == 3) return PAUSE_EXIT;
            }
        }

        mouseWasPressed = mouseIsPressed;
        return PAUSE_NOTHING;
    }

    void update(float dt) {

    }

    void draw(sf::RenderWindow& window) {

        window.draw(backgroundOverlay);


        for (int i = 0; i < 4; i++) {
            window.draw(optionBoxes[i]);
            window.draw(options[i]);
        }
    }
};




class GameOver {
private:
    sf::Text titleText;
    sf::Text scoreLabel;
    sf::Text timeLabel;
    bool isVictory;
    sf::Text messageText;
    sf::Sprite timeDigits[4];
    float finalTime;
    sf::Texture numeralTextures[10];
    sf::Sprite scoreSprites[6];
    int currentScore;

    sf::Text options[3];
    sf::RectangleShape optionBoxes[3];
    string labels[3] = { "Play Again", "Main Menu", "Exit Game" };

    sf::RectangleShape backgroundOverlay;
    sf::Font font;

    sf::Color idleColor;
    sf::Color hoverColor;
    sf::Color orangeColor;

    int hoveredIndex;
    float windowWidth;
    float windowHeight;
    bool fontLoaded;
    bool mouseWasPressed;

    void initializeOptions() {
        float centerX = windowWidth * 0.5f;
        float startY = windowHeight * 0.58f;
        float spacing = 90.0f;
        float boxWidth = 400.0f;
        float boxHeight = 70.0f;


        titleText.setFont(font);
        titleText.setString("GAME OVER");
        titleText.setCharacterSize(80);
        titleText.setFillColor(sf::Color::Red);
        sf::FloatRect titleBounds = titleText.getLocalBounds();
        titleText.setPosition(centerX - titleBounds.width / 2.0f, windowHeight * 0.15f);


        scoreLabel.setFont(font);
        scoreLabel.setString("Final Score:");
        scoreLabel.setCharacterSize(36);
        scoreLabel.setFillColor(sf::Color::White);
        sf::FloatRect labelBounds = scoreLabel.getLocalBounds();
        scoreLabel.setPosition(centerX - labelBounds.width / 2.0f, windowHeight * 0.28f);

        messageText.setFont(font);
        messageText.setString("");
        messageText.setCharacterSize(48);
        messageText.setFillColor(sf::Color::White);


        timeLabel.setFont(font);
        timeLabel.setString("Time:");
        timeLabel.setCharacterSize(36);
        timeLabel.setFillColor(sf::Color::White);
        sf::FloatRect timeLabelBounds = timeLabel.getLocalBounds();
        timeLabel.setPosition(centerX - timeLabelBounds.width / 2.0f, windowHeight * 0.42f);


        for (int i = 0; i < 10; i++) {
            string filename = "numeral" + std::to_string(i) + ".png";
            if (!numeralTextures[i].loadFromFile(filename)) {
                cout << "Warning: Could not load " << filename << endl;
            }
        }

        for (int i = 0; i < 3; i++) {
            options[i].setFont(font);
            options[i].setString(labels[i]);
            options[i].setCharacterSize(36);
            options[i].setFillColor(idleColor);

            sf::FloatRect textBounds = options[i].getLocalBounds();
            float textX = centerX - textBounds.width / 2.0f - textBounds.left;
            float textY = startY + i * spacing + (boxHeight - textBounds.height) / 2.0f - textBounds.top;
            options[i].setPosition(textX, textY);

            optionBoxes[i].setSize(sf::Vector2f(boxWidth, boxHeight));
            optionBoxes[i].setPosition(centerX - boxWidth / 2.0f, startY + i * spacing);
            optionBoxes[i].setFillColor(sf::Color(0, 0, 0, 150));
            optionBoxes[i].setOutlineThickness(2);
            optionBoxes[i].setOutlineColor(sf::Color(100, 100, 100));
        }
    }

    void updateHover(sf::Vector2f mousePos) {
        hoveredIndex = -1;
        for (int i = 0; i < 3; i++) {
            if (optionBoxes[i].getGlobalBounds().contains(mousePos)) {
                hoveredIndex = i;
                options[i].setFillColor(hoverColor);
                optionBoxes[i].setOutlineColor(orangeColor);
                optionBoxes[i].setOutlineThickness(3);
            }
            else {
                options[i].setFillColor(idleColor);
                optionBoxes[i].setOutlineColor(sf::Color(100, 100, 100));
                optionBoxes[i].setOutlineThickness(2);
            }
        }
    }

public:
public:
    GameOver() : hoveredIndex(-1), fontLoaded(false), windowWidth(1920.0f), windowHeight(1080.0f), mouseWasPressed(false), currentScore(0), finalTime(0.0f), isVictory(false) {
        idleColor = sf::Color(255, 200, 150);
        hoverColor = sf::Color::White;
        orangeColor = sf::Color(255, 100, 50);

        backgroundOverlay.setSize(sf::Vector2f(windowWidth, windowHeight));
        backgroundOverlay.setFillColor(sf::Color(0, 0, 0, 200));
    }

    bool loadFont(const string& fontPath) {
        if (font.loadFromFile(fontPath)) {
            fontLoaded = true;
            if (windowWidth > 0 && windowHeight > 0) initializeOptions();
            return true;
        }
        if (font.loadFromFile("arial.ttf")) {
            fontLoaded = true;
            if (windowWidth > 0 && windowHeight > 0) initializeOptions();
            return true;
        }
        return false;
    }

    void setVictory(bool victory) {
        isVictory = victory;

        if (isVictory) {

            titleText.setString("VICTORY!");
            titleText.setFillColor(sf::Color(255, 215, 0));


            messageText.setString("Mission Accomplished!");
            messageText.setFillColor(sf::Color(50, 255, 100));
        }
        else {

            titleText.setString("GAME OVER");
            titleText.setFillColor(sf::Color::Red);


            messageText.setString("");
        }


        sf::FloatRect titleBounds = titleText.getLocalBounds();
        titleText.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
        titleText.setPosition(windowWidth * 0.5f, windowHeight * 0.15f);


        sf::FloatRect msgBounds = messageText.getLocalBounds();
        messageText.setOrigin(msgBounds.width / 2.0f, msgBounds.height / 2.0f);
        messageText.setPosition(windowWidth * 0.5f, windowHeight * 0.23f);
    }

    void initialize(float width, float height) {
        windowWidth = width;
        windowHeight = height;
        backgroundOverlay.setSize(sf::Vector2f(width, height));
        if (fontLoaded) initializeOptions();
    }

    void setScore(int score) {
        currentScore = score;
        string scoreStr = std::to_string(score);

        float digitWidth = 30.0f;
        float totalWidth = scoreStr.length() * digitWidth;
        float startX = windowWidth * 0.5f - totalWidth / 2.0f;
        float startY = windowHeight * 0.34f;

        for (int i = 0; i < (int)scoreStr.length(); i++) {
            int digit = scoreStr[i] - '0';
            if (i < 6) {
                scoreSprites[i].setTexture(numeralTextures[digit]);
                scoreSprites[i].setScale(1.2f, 1.2f);
                scoreSprites[i].setPosition(startX + i * 40.0f, startY);
            }
        }
    }

    void setTime(float timeInSeconds) {
        finalTime = timeInSeconds;


        int totalSeconds = (int)timeInSeconds;
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;

        if (minutes > 99) minutes = 99;

        int minute1 = minutes / 10;
        int minute2 = minutes % 10;
        int second1 = seconds / 10;
        int second2 = seconds % 10;

        float centerX = windowWidth * 0.5f;
        float startY = windowHeight * 0.48f;
        float digitSpacing = 40.0f;


        timeDigits[0].setTexture(numeralTextures[minute1]);
        timeDigits[0].setScale(1.2f, 1.2f);
        timeDigits[0].setPosition(centerX - 2.0f * digitSpacing, startY);

        timeDigits[1].setTexture(numeralTextures[minute2]);
        timeDigits[1].setScale(1.2f, 1.2f);
        timeDigits[1].setPosition(centerX - 1.0f * digitSpacing, startY);

        timeDigits[2].setTexture(numeralTextures[second1]);
        timeDigits[2].setScale(1.2f, 1.2f);
        timeDigits[2].setPosition(centerX + 0.5f * digitSpacing, startY);

        timeDigits[3].setTexture(numeralTextures[second2]);
        timeDigits[3].setScale(1.2f, 1.2f);
        timeDigits[3].setPosition(centerX + 1.5f * digitSpacing, startY);
    }

    int handleInput(sf::RenderWindow& window) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        updateHover(mousePos);

        bool mouseIsPressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
        if (mouseIsPressed && !mouseWasPressed) {
            mouseWasPressed = true;
            if (hoveredIndex == 0) return PAUSE_RESTART;
            if (hoveredIndex == 1) return PAUSE_MAIN_MENU;
            if (hoveredIndex == 2) return PAUSE_EXIT;
        }
        mouseWasPressed = mouseIsPressed;
        return PAUSE_NOTHING;
    }

    void update(float dt) {

    }

    void draw(sf::RenderWindow& window) {
        window.draw(backgroundOverlay);
        window.draw(titleText);


        if (isVictory && messageText.getString() != "") {
            window.draw(messageText);
        }

        window.draw(scoreLabel);


        string scoreStr = std::to_string(currentScore);
        for (int i = 0; i < (int)scoreStr.length() && i < 6; i++) {
            window.draw(scoreSprites[i]);
        }

        window.draw(timeLabel);
        for (int i = 0; i < 4; i++) {
            window.draw(timeDigits[i]);
        }


        float centerX = windowWidth * 0.5f;
        float colonY = windowHeight * 0.48f + 10.0f;
        sf::CircleShape colonDot1(3.0f);
        colonDot1.setFillColor(sf::Color::White);
        colonDot1.setPosition(centerX - 5.0f, colonY);
        window.draw(colonDot1);

        sf::CircleShape colonDot2(3.0f);
        colonDot2.setFillColor(sf::Color::White);
        colonDot2.setPosition(centerX - 5.0f, colonY + 15.0f);
        window.draw(colonDot2);

        for (int i = 0; i < 3; i++) {
            window.draw(optionBoxes[i]);
            window.draw(options[i]);
        }
    }
};




class HighScoreScreen {
private:
    sf::Font font;
    sf::Text titleText;
    sf::Text backText;
    sf::Text scrollText;
    sf::Text headers[4];
    sf::Text nameEntries[10];

    string names[50];
    int scores[50];
    int times[50];
    int count;
    int scrollOffset;


    sf::Texture numeralTextures[10];
    sf::Texture dotTexture;
    sf::Sprite rankSprites[10][3];
    sf::Sprite scoreSprites[10][6];
    sf::Sprite timeSprites[10][5];

public:
    HighScoreScreen() : count(0), scrollOffset(0) {
        for (int i = 0; i < 50; i++) {
            names[i] = "";
            scores[i] = 0;
            times[i] = 0;
        }
    }

    bool loadFont(const string& fontPath) {
        if (!font.loadFromFile(fontPath)) {
            return false;
        }


        for (int i = 0; i < 10; i++) {
            string filename = "numeral" + std::to_string(i) + ".png";
            if (!numeralTextures[i].loadFromFile(filename)) {
                cout << "Warning: Could not load " << filename << endl;
            }
        }

        return true;
    }

    void loadScores() {
        count = 0;
        for (int i = 0; i < 50; i++) {
            names[i] = "";
            scores[i] = 0;
            times[i] = 0;
        }

        ifstream fileIn("highscores.txt");
        if (fileIn) {
            string line;
            while (count < 50 && getline(fileIn, line)) {


                size_t lastSpacePos = line.rfind(' ');
                if (lastSpacePos != string::npos) {

                    size_t secondLastSpace = line.rfind(' ', lastSpacePos - 1);
                    if (secondLastSpace != string::npos) {

                        names[count] = line.substr(0, secondLastSpace);


                        scores[count] = stoi(line.substr(secondLastSpace + 1, lastSpacePos - secondLastSpace - 1));


                        times[count] = stoi(line.substr(lastSpacePos + 1));

                        count++;
                    }
                }
            }
            fileIn.close();
        }
    }

    void initialize() {
        scrollOffset = 0;

        titleText.setFont(font);
        titleText.setString("HIGH SCORES");
        titleText.setCharacterSize(80);
        titleText.setFillColor(sf::Color::Cyan);
        sf::FloatRect titleBounds = titleText.getLocalBounds();
        titleText.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
        titleText.setPosition(960.0f, 100.0f);


        headers[0].setFont(font);
        headers[0].setString("RANK");
        headers[0].setCharacterSize(35);
        headers[0].setFillColor(sf::Color(255, 215, 0));
        headers[0].setPosition(400.0f, 200.0f);

        headers[1].setFont(font);
        headers[1].setString("PLAYER");
        headers[1].setCharacterSize(35);
        headers[1].setFillColor(sf::Color(255, 215, 0));
        headers[1].setPosition(650.0f, 200.0f);

        headers[2].setFont(font);
        headers[2].setString("SCORE");
        headers[2].setCharacterSize(35);
        headers[2].setFillColor(sf::Color(255, 215, 0));
        headers[2].setPosition(1000.0f, 200.0f);


        headers[3].setFont(font);
        headers[3].setString("TIME");
        headers[3].setCharacterSize(35);
        headers[3].setFillColor(sf::Color(255, 215, 0));
        headers[3].setPosition(1300.0f, 200.0f);


        scrollText.setFont(font);
        scrollText.setString("Use UP/DOWN arrows to scroll | Press ESC to return");
        scrollText.setCharacterSize(25);
        scrollText.setFillColor(sf::Color(150, 150, 255));
        sf::FloatRect scrollBounds = scrollText.getLocalBounds();
        scrollText.setOrigin(scrollBounds.width / 2.0f, scrollBounds.height / 2.0f);
        scrollText.setPosition(960.0f, 950.0f);

        backText.setFont(font);
        backText.setString("Press ESC to return");
        backText.setCharacterSize(30);
        backText.setFillColor(sf::Color(255, 100, 100));
        sf::FloatRect backBounds = backText.getLocalBounds();
        backText.setOrigin(backBounds.width / 2.0f, backBounds.height / 2.0f);
        backText.setPosition(960.0f, 1000.0f);

        updateDisplay();
    }

    void handleInput(sf::Keyboard::Key key) {
        if (key == sf::Keyboard::Up) {
            if (scrollOffset > 0) {
                scrollOffset--;
                updateDisplay();
            }
        }
        else if (key == sf::Keyboard::Down) {

            if (scrollOffset < count - 10 && count > 10) {
                scrollOffset++;
                updateDisplay();
            }
        }
    }

    void updateDisplay() {

        for (int i = 0; i < 10; i++) {
            int dataIndex = scrollOffset + i;
            float rowY = 270.0f + i * 65.0f;

            if (dataIndex < count) {

                int rankNumber = dataIndex + 1;
                string rankStr = std::to_string(rankNumber);


                for (int d = 0; d < (int)rankStr.length() && d < 2; d++) {
                    int digit = rankStr[d] - '0';
                    rankSprites[i][d].setTexture(numeralTextures[digit]);
                    rankSprites[i][d].setScale(0.8f, 0.8f);
                    rankSprites[i][d].setPosition(400.0f + d * 25.0f, rowY);
                }


                nameEntries[i].setFont(font);
                nameEntries[i].setString(names[dataIndex]);
                nameEntries[i].setCharacterSize(28);
                nameEntries[i].setFillColor(sf::Color::White);
                nameEntries[i].setPosition(650.0f, rowY);


                if (scores[dataIndex] > 0) {
                    string scoreStr = std::to_string(scores[dataIndex]);
                    float startX = 1000.0f;

                    for (int j = 0; j < (int)scoreStr.length() && j < 6; j++) {
                        int digit = scoreStr[j] - '0';
                        scoreSprites[i][j].setTexture(numeralTextures[digit]);
                        scoreSprites[i][j].setScale(0.9f, 0.9f);
                        scoreSprites[i][j].setPosition(startX + j * 30.0f, rowY);
                    }
                }
                else {

                    scoreSprites[i][0].setTexture(numeralTextures[0]);
                    scoreSprites[i][0].setScale(0.9f, 0.9f);
                    scoreSprites[i][0].setPosition(1000.0f, rowY);
                }

                int totalSeconds = times[dataIndex];
                int minutes = totalSeconds / 60;
                int seconds = totalSeconds % 60;

                if (minutes > 99) minutes = 99;

                int minute1 = minutes / 10;
                int minute2 = minutes % 10;
                int second1 = seconds / 10;
                int second2 = seconds % 10;

                float timeStartX = 1300.0f;
                float digitSpacing = 25.0f;


                timeSprites[i][0].setTexture(numeralTextures[minute1]);
                timeSprites[i][0].setScale(0.7f, 0.7f);
                timeSprites[i][0].setPosition(timeStartX, rowY);

                timeSprites[i][1].setTexture(numeralTextures[minute2]);
                timeSprites[i][1].setScale(0.7f, 0.7f);
                timeSprites[i][1].setPosition(timeStartX + digitSpacing, rowY);


                timeSprites[i][2].setTexture(numeralTextures[second1]);
                timeSprites[i][2].setScale(0.7f, 0.7f);
                timeSprites[i][2].setPosition(timeStartX + digitSpacing * 2.5f, rowY);

                timeSprites[i][3].setTexture(numeralTextures[second2]);
                timeSprites[i][3].setScale(0.7f, 0.7f);
                timeSprites[i][3].setPosition(timeStartX + digitSpacing * 3.5f, rowY);
            }
            else if (i == 0 && count == 0) {

                nameEntries[i].setFont(font);
                nameEntries[i].setString("No high scores yet - be the first!");
                nameEntries[i].setCharacterSize(32);
                nameEntries[i].setFillColor(sf::Color(150, 150, 255));
                nameEntries[i].setPosition(650.0f, rowY);


                for (int d = 0; d < 2; d++) {
                    rankSprites[i][d].setScale(0.0f, 0.0f);
                }
                for (int j = 0; j < 6; j++) {
                    scoreSprites[i][j].setScale(0.0f, 0.0f);
                }
                for (int j = 0; j < 5; j++) {
                    timeSprites[i][j].setScale(0.0f, 0.0f);
                }
            }
            else {

                nameEntries[i].setString("");
                for (int d = 0; d < 2; d++) {
                    rankSprites[i][d].setScale(0.0f, 0.0f);
                }
                for (int j = 0; j < 6; j++) {
                    scoreSprites[i][j].setScale(0.0f, 0.0f);
                }
                for (int j = 0; j < 5; j++) {
                    timeSprites[i][j].setScale(0.0f, 0.0f);
                }
            }
        }
    }

    void draw(sf::RenderWindow& window) {

        static bool starsInit = false;
        static sf::CircleShape stars[200];
        if (!starsInit) {
            srand((unsigned int)time(nullptr) + 100);
            for (int i = 0; i < 200; i++) {
                float r = 1.0f + (rand() % 3);
                stars[i].setRadius(r);
                stars[i].setPosition((float)(rand() % 1920), (float)(rand() % 1080));
                int b = 150 + rand() % 100;
                stars[i].setFillColor(sf::Color((sf::Uint8)b, (sf::Uint8)b, (sf::Uint8)b));
            }
            starsInit = true;
        }

        for (int i = 0; i < 200; i++) {
            window.draw(stars[i]);
        }

        window.draw(titleText);
        for (int i = 0; i < 4; i++) {
            window.draw(headers[i]);
        }


        for (int i = 0; i < 10; i++) {
            int dataIndex = scrollOffset + i;
            float rowY = 270.0f + i * 65.0f;

            if (dataIndex < count) {

                string rankStr = std::to_string(dataIndex + 1);
                for (int d = 0; d < (int)rankStr.length() && d < 2; d++) {
                    window.draw(rankSprites[i][d]);
                }


                sf::Text dotText;
                dotText.setFont(font);
                dotText.setString(".");
                dotText.setCharacterSize(28);
                dotText.setFillColor(sf::Color::White);
                dotText.setPosition(400.0f + rankStr.length() * 25.0f, rowY);
                window.draw(dotText);


                window.draw(nameEntries[i]);


                if (scores[dataIndex] > 0) {
                    string scoreStr = std::to_string(scores[dataIndex]);
                    for (int j = 0; j < (int)scoreStr.length() && j < 6; j++) {
                        window.draw(scoreSprites[i][j]);
                    }
                }
                else {
                    window.draw(scoreSprites[i][0]);
                }


                for (int j = 0; j < 4; j++) {
                    window.draw(timeSprites[i][j]);
                }


                float timeStartX = 1300.0f;
                float digitSpacing = 25.0f;
                sf::CircleShape colonDot1(2.5f);
                colonDot1.setFillColor(sf::Color::White);
                colonDot1.setPosition(timeStartX + digitSpacing * 2.1f, rowY + 8.0f);
                window.draw(colonDot1);

                sf::CircleShape colonDot2(2.5f);
                colonDot2.setFillColor(sf::Color::White);
                colonDot2.setPosition(timeStartX + digitSpacing * 2.1f, rowY + 18.0f);
                window.draw(colonDot2);
            }
            else if (dataIndex >= count && i == 0 && count == 0) {

                window.draw(nameEntries[i]);
            }
        }

        window.draw(scrollText);
    }
};




void saveHighScore(const string& playerName, int score, float timeInSeconds) {
    const int MAX_SCORES = 50;

    try {

        string* names = new string[MAX_SCORES];
        int* scores = new int[MAX_SCORES]();
        int* times = new int[MAX_SCORES]();
        int count = 0;


        try {
            ifstream fileIn("highscores.txt");
            if (!fileIn) {
                throw FileLoadException("highscores.txt");
            }

            string line;
            while (count < MAX_SCORES && getline(fileIn, line)) {
                size_t lastSpacePos = line.rfind(' ');
                if (lastSpacePos != string::npos) {
                    size_t secondLastSpace = line.rfind(' ', lastSpacePos - 1);
                    if (secondLastSpace != string::npos) {
                        names[count] = line.substr(0, secondLastSpace);
                        scores[count] = stoi(line.substr(secondLastSpace + 1, lastSpacePos - secondLastSpace - 1));
                        times[count] = stoi(line.substr(lastSpacePos + 1));
                        count++;
                    }
                }
            }
            fileIn.close();
        }
        catch (const FileLoadException& e) {
            cout << "Note: " << e.what() << " (creating new file)" << endl;
        }


        int intTime = (int)timeInSeconds;


        bool scoreAdded = false;
        for (int i = 0; i < MAX_SCORES; i++) {
            if (score > scores[i] || (score == scores[i] && intTime < times[i])) {

                for (int j = MAX_SCORES - 1; j > i; j--) {
                    names[j] = names[j - 1];
                    scores[j] = scores[j - 1];
                    times[j] = times[j - 1];
                }
                names[i] = playerName;
                scores[i] = score;
                times[i] = intTime;
                scoreAdded = true;
                if (count < MAX_SCORES) count++;
                break;
            }
        }

        if (!scoreAdded && count < MAX_SCORES) {
            names[count] = playerName;
            scores[count] = score;
            times[count] = intTime;
            count++;
        }


        try {
            ofstream fileOut("highscores.txt");
            if (!fileOut) {
                throw FileLoadException("highscores.txt (write)");
            }

            for (int i = 0; i < count; i++) {
                fileOut << names[i] << " " << scores[i] << " " << times[i] << endl;
            }
            fileOut.close();
            cout << "High score saved successfully!" << endl;
        }
        catch (const FileLoadException& e) {
            cout << "ERROR: " << e.what() << endl;
        }


        delete[] names;
        delete[] scores;
        delete[] times;

    }
    catch (const exception& e) {
        cout << "Unexpected error in saveHighScore: " << e.what() << endl;
    }
}




class ShipSelection {
private:
    sf::Text titleText;
    sf::Text instructionText;
    sf::Font font;
    sf::RectangleShape backgroundOverlay;


    sf::Texture shipTextures[4];
    sf::Sprite shipSprites[4];
    sf::RectangleShape shipBoxes[4];
    sf::Text shipLabels[4];

    int selectedShip;
    string shipColors[4];

    float windowWidth;
    float windowHeight;


    sf::Clock glowClock;
    float glowIntensity;

public:
    ShipSelection()
        : windowWidth(1920.0f),
        windowHeight(1080.0f),
        selectedShip(0),
        glowIntensity(0.0f)
    {
        shipColors[0] = "Red";
        shipColors[1] = "Blue";
        shipColors[2] = "Green";
        shipColors[3] = "Orange";
    }

    bool loadFont(const string& fontPath) {
        if (!font.loadFromFile(fontPath)) {
            return false;
        }
        return true;
    }

    bool loadShipTextures() {
        if (!shipTextures[0].loadFromFile("playerShip1_red.png")) return false;
        if (!shipTextures[1].loadFromFile("playerShip1_blue.png")) return false;
        if (!shipTextures[2].loadFromFile("playerShip1_green.png")) return false;
        if (!shipTextures[3].loadFromFile("playerShip1_orange.png")) return false;
        return true;
    }

    void initialize(float width, float height) {
        windowWidth = width;
        windowHeight = height;


        backgroundOverlay.setSize(sf::Vector2f(width, height));
        backgroundOverlay.setFillColor(sf::Color(5, 5, 25, 255));


        titleText.setFont(font);
        titleText.setString("CHOOSE YOUR SHIP");
        titleText.setCharacterSize(72);
        titleText.setFillColor(sf::Color::Cyan);
        sf::FloatRect titleBounds = titleText.getLocalBounds();
        titleText.setOrigin(titleBounds.left + titleBounds.width / 2.0f, titleBounds.top + titleBounds.height / 2.0f);
        titleText.setPosition(width * 0.5f, height * 0.15f);


        instructionText.setFont(font);
        instructionText.setString("Use LEFT/RIGHT arrows to select  |  Press ENTER to confirm");
        instructionText.setCharacterSize(28);
        instructionText.setFillColor(sf::Color(200, 200, 200));
        sf::FloatRect instBounds = instructionText.getLocalBounds();
        instructionText.setOrigin(instBounds.left + instBounds.width / 2.0f, instBounds.top + instBounds.height / 2.0f);
        instructionText.setPosition(width * 0.5f, height * 0.85f);


        float shipY = height * 0.5f;
        float spacing = width / 5.0f;
        float startX = spacing;

        for (int i = 0; i < 4; i++) {

            shipSprites[i].setTexture(shipTextures[i]);
            sf::FloatRect bounds = shipSprites[i].getLocalBounds();
            shipSprites[i].setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
            shipSprites[i].setPosition(startX + i * spacing, shipY);
            shipSprites[i].setScale(2.0f, 2.0f);


            shipBoxes[i].setSize(sf::Vector2f(250.0f, 300.0f));
            shipBoxes[i].setOrigin(125.0f, 150.0f);
            shipBoxes[i].setPosition(startX + i * spacing, shipY);
            shipBoxes[i].setFillColor(sf::Color(0, 0, 0, 0));
            shipBoxes[i].setOutlineThickness(3.0f);
            shipBoxes[i].setOutlineColor(sf::Color(100, 100, 100, 150));


            shipLabels[i].setFont(font);
            shipLabels[i].setString(shipColors[i]);
            shipLabels[i].setCharacterSize(36);
            shipLabels[i].setFillColor(sf::Color::White);
            sf::FloatRect labelBounds = shipLabels[i].getLocalBounds();
            shipLabels[i].setOrigin(labelBounds.left + labelBounds.width / 2.0f, labelBounds.top + labelBounds.height / 2.0f);
            shipLabels[i].setPosition(startX + i * spacing, shipY + 180.0f);
        }
    }

    void update(float dt) {

        glowIntensity = (sin(glowClock.getElapsedTime().asSeconds() * 3.0f) + 1.0f) / 2.0f;


        for (int i = 0; i < 4; i++) {
            if (i == selectedShip) {

                sf::Uint8 glowAlpha = (sf::Uint8)(150 + glowIntensity * 105);
                sf::Color glowColor = sf::Color::Cyan;
                glowColor.a = glowAlpha;
                shipBoxes[i].setOutlineColor(glowColor);
                shipBoxes[i].setOutlineThickness(5.0f + glowIntensity * 3.0f);


                float scale = 2.0f + glowIntensity * 0.2f;
                shipSprites[i].setScale(scale, scale);
            }
            else {
                shipBoxes[i].setOutlineColor(sf::Color(100, 100, 100, 150));
                shipBoxes[i].setOutlineThickness(3.0f);
                shipSprites[i].setScale(2.0f, 2.0f);
            }
        }
    }

    void handleInput(sf::Keyboard::Key key) {
        if (key == sf::Keyboard::Left) {
            selectedShip = (selectedShip + 3) % 4;
        }
        else if (key == sf::Keyboard::Right) {
            selectedShip = (selectedShip + 1) % 4;
        }
    }

    string getSelectedShipColor() const {
        return shipColors[selectedShip];
    }

    void draw(sf::RenderWindow& window) {
        window.draw(backgroundOverlay);


        drawStars(window);

        window.draw(titleText);
        window.draw(instructionText);


        for (int i = 0; i < 4; i++) {
            window.draw(shipBoxes[i]);
            window.draw(shipSprites[i]);
            window.draw(shipLabels[i]);
        }
    }

private:
    void drawStars(sf::RenderWindow& win) {
        static bool init = false;
        static sf::CircleShape stars[150];

        if (!init) {
            srand((unsigned int)time(nullptr) + 999);
            for (int i = 0; i < 150; i++) {
                float r = 1.0f + (rand() % 3);
                stars[i].setRadius(r);
                stars[i].setPosition((float)(rand() % 1920), (float)(rand() % 1080));
                int b = 150 + rand() % 100;
                stars[i].setFillColor(sf::Color((sf::Uint8)b, (sf::Uint8)b, (sf::Uint8)b));
            }
            init = true;
        }

        for (int i = 0; i < 150; i++) {
            win.draw(stars[i]);
        }
    }
};




class VictoryScreen {
private:
    sf::Text titleText;
    sf::Text messageText;
    sf::Text continueText;
    sf::Font font;
    sf::RectangleShape backgroundOverlay;
    float windowWidth;
    float windowHeight;

public:
    VictoryScreen() : windowWidth(1920.0f), windowHeight(1080.0f) {}

    bool loadFont(const string& fontPath) {
        if (!font.loadFromFile(fontPath)) {
            return false;
        }
        return true;
    }

    void initialize(float width, float height) {
        windowWidth = width;
        windowHeight = height;


        backgroundOverlay.setSize(sf::Vector2f(width, height));
        backgroundOverlay.setFillColor(sf::Color(0, 0, 0, 180));


        titleText.setFont(font);
        titleText.setString("CONGRATULATIONS!");
        titleText.setCharacterSize(80);
        titleText.setFillColor(sf::Color(255, 215, 0));
        sf::FloatRect titleBounds = titleText.getLocalBounds();
        titleText.setOrigin(titleBounds.left + titleBounds.width / 2.0f, titleBounds.top + titleBounds.height / 2.0f);
        titleText.setPosition(width * 0.5f, height * 0.35f);


        messageText.setFont(font);
        messageText.setString("You advance to the next level!");
        messageText.setCharacterSize(48);
        messageText.setFillColor(sf::Color::White);
        sf::FloatRect msgBounds = messageText.getLocalBounds();
        messageText.setOrigin(msgBounds.left + msgBounds.width / 2.0f, msgBounds.top + msgBounds.height / 2.0f);
        messageText.setPosition(width * 0.5f, height * 0.5f);


        continueText.setFont(font);
        continueText.setString("Press ENTER to return to menu");
        continueText.setCharacterSize(32);
        continueText.setFillColor(sf::Color(200, 200, 200));
        sf::FloatRect contBounds = continueText.getLocalBounds();
        continueText.setOrigin(contBounds.left + contBounds.width / 2.0f, contBounds.top + contBounds.height / 2.0f);
        continueText.setPosition(width * 0.5f, height * 0.7f);
    }

    void update(float dt) {

    }

    void draw(sf::RenderWindow& window) {
        window.draw(backgroundOverlay);
        window.draw(titleText);
        window.draw(messageText);
        window.draw(continueText);
    }
};


class LevelSelection {
private:
    sf::Text titleText;
    sf::Text backText;
    sf::Font font;
    sf::RectangleShape backgroundOverlay;

    sf::Text levelOptions[3];
    sf::RectangleShape levelBoxes[3];
    string labels[3] = { "Level 1", "Level 2", "Boss Level" };

    sf::Color idleColor;
    sf::Color hoverColor;
    sf::Color accentColor;

    int hoveredIndex;
    float windowWidth;
    float windowHeight;
    bool fontLoaded;
    bool mouseWasPressed;

public:
    LevelSelection() : hoveredIndex(-1), fontLoaded(false), windowWidth(1920.0f), windowHeight(1080.0f), mouseWasPressed(false) {
        idleColor = sf::Color(255, 200, 150);
        hoverColor = sf::Color::White;
        accentColor = sf::Color(0, 200, 255);

        backgroundOverlay.setSize(sf::Vector2f(windowWidth, windowHeight));
        backgroundOverlay.setFillColor(sf::Color(5, 5, 25, 255));
    }

    bool loadFont(const string& fontPath) {
        if (font.loadFromFile(fontPath)) {
            fontLoaded = true;
            return true;
        }
        if (font.loadFromFile("arial.ttf")) {
            fontLoaded = true;
            return true;
        }
        return false;
    }

    void initialize(float width, float height) {
        windowWidth = width;
        windowHeight = height;
        backgroundOverlay.setSize(sf::Vector2f(width, height));


        titleText.setFont(font);
        titleText.setString("SELECT LEVEL");
        titleText.setCharacterSize(80);
        titleText.setFillColor(sf::Color::Cyan);
        sf::FloatRect titleBounds = titleText.getLocalBounds();
        titleText.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
        titleText.setPosition(width * 0.5f, height * 0.2f);


        backText.setFont(font);
        backText.setString("Press ESC to return");
        backText.setCharacterSize(30);
        backText.setFillColor(sf::Color(150, 150, 255));
        sf::FloatRect backBounds = backText.getLocalBounds();
        backText.setOrigin(backBounds.width / 2.0f, backBounds.height / 2.0f);
        backText.setPosition(width * 0.5f, height * 0.9f);


        float centerX = width * 0.5f;
        float startY = height * 0.4f;
        float spacing = 120.0f;
        float boxWidth = 500.0f;
        float boxHeight = 80.0f;

        for (int i = 0; i < 3; i++) {
            levelOptions[i].setFont(font);
            levelOptions[i].setString(labels[i]);
            levelOptions[i].setCharacterSize(42);
            levelOptions[i].setFillColor(idleColor);

            sf::FloatRect textBounds = levelOptions[i].getLocalBounds();
            float textX = centerX - textBounds.width / 2.0f - textBounds.left;
            float textY = startY + i * spacing + (boxHeight - textBounds.height) / 2.0f - textBounds.top;
            levelOptions[i].setPosition(textX, textY);

            levelBoxes[i].setSize(sf::Vector2f(boxWidth, boxHeight));
            levelBoxes[i].setPosition(centerX - boxWidth / 2.0f, startY + i * spacing);
            levelBoxes[i].setFillColor(sf::Color(0, 0, 0, 150));
            levelBoxes[i].setOutlineThickness(3);
            levelBoxes[i].setOutlineColor(sf::Color(100, 100, 100));
        }
    }

    void update(float dt) {

    }

    int handleInput(sf::RenderWindow& window) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));


        hoveredIndex = -1;
        for (int i = 0; i < 3; i++) {
            if (levelBoxes[i].getGlobalBounds().contains(mousePos)) {
                hoveredIndex = i;
                levelOptions[i].setFillColor(hoverColor);
                levelBoxes[i].setOutlineColor(accentColor);
                levelBoxes[i].setOutlineThickness(5);
            }
            else {
                levelOptions[i].setFillColor(idleColor);
                levelBoxes[i].setOutlineColor(sf::Color(100, 100, 100));
                levelBoxes[i].setOutlineThickness(3);
            }
        }


        bool mouseIsPressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
        if (mouseIsPressed && !mouseWasPressed && hoveredIndex >= 0) {
            mouseWasPressed = true;
            return hoveredIndex + 1;
        }
        mouseWasPressed = mouseIsPressed;
        return -1;
    }

    void draw(sf::RenderWindow& window) {
        window.draw(backgroundOverlay);


        static bool starsInit = false;
        static sf::CircleShape stars[150];
        if (!starsInit) {
            srand((unsigned int)time(nullptr) + 500);
            for (int i = 0; i < 150; i++) {
                float r = 1.0f + (rand() % 3);
                stars[i].setRadius(r);
                stars[i].setPosition((float)(rand() % 1920), (float)(rand() % 1080));
                int b = 150 + rand() % 100;
                stars[i].setFillColor(sf::Color((sf::Uint8)b, (sf::Uint8)b, (sf::Uint8)b));
            }
            starsInit = true;
        }
        for (int i = 0; i < 150; i++) {
            window.draw(stars[i]);
        }

        window.draw(titleText);
        for (int i = 0; i < 3; i++) {
            window.draw(levelBoxes[i]);
            window.draw(levelOptions[i]);
        }
        window.draw(backText);
    }
};



class CreditsScreen {
private:
    sf::Text titleText;
    sf::Text creditsText[15];
    sf::Text backText;
    sf::Font font;
    sf::RectangleShape backgroundOverlay;
    float windowWidth;
    float windowHeight;

public:
    CreditsScreen() : windowWidth(1920.0f), windowHeight(1080.0f) {}

    bool loadFont(const string& fontPath) {
        if (font.loadFromFile(fontPath)) {
            return true;
        }
        if (font.loadFromFile("arial.ttf")) {
            return true;
        }
        return false;
    }

    void initialize(float width, float height) {
        windowWidth = width;
        windowHeight = height;

        backgroundOverlay.setSize(sf::Vector2f(width, height));
        backgroundOverlay.setFillColor(sf::Color(5, 5, 25, 255));


        titleText.setFont(font);
        titleText.setString("CREDITS");
        titleText.setCharacterSize(80);
        titleText.setFillColor(sf::Color::Cyan);
        sf::FloatRect titleBounds = titleText.getLocalBounds();
        titleText.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
        titleText.setPosition(width * 0.5f, 100.0f);


        string creditLines[15] = {
            "",
            "Programmed by",
            "Mitul Dial & Arham Manzoor",
            "",
            "GAME DESIGN BY",
            "Mitul Dial",
            "",
            "STORY BY",
            "Arham Manzoor",
            "",
            "PRODUCED BY",
            "Mitul Dial & Arham Manzoor",
            ""
        };

        float startY = 220.0f;
        float lineSpacing = 50.0f;

        for (int i = 0; i < 15; i++) {
            creditsText[i].setFont(font);
            creditsText[i].setString(creditLines[i]);


            if (creditLines[i] == "GAME DEVELOPMENT" ||
                creditLines[i] == "GAME DESIGN" ||
                creditLines[i] == "ASSETS & RESOURCES" ||
                creditLines[i] == "SPECIAL THANKS") {
                creditsText[i].setCharacterSize(40);
                creditsText[i].setFillColor(sf::Color(255, 215, 0));
            }
            else {
                creditsText[i].setCharacterSize(30);
                creditsText[i].setFillColor(sf::Color::White);
            }

            sf::FloatRect bounds = creditsText[i].getLocalBounds();
            creditsText[i].setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
            creditsText[i].setPosition(width * 0.5f, startY + i * lineSpacing);
        }


        backText.setFont(font);
        backText.setString("Press ESC to return");
        backText.setCharacterSize(30);
        backText.setFillColor(sf::Color(150, 150, 255));
        sf::FloatRect backBounds = backText.getLocalBounds();
        backText.setOrigin(backBounds.width / 2.0f, backBounds.height / 2.0f);
        backText.setPosition(width * 0.5f, height * 0.95f);
    }

    void update(float dt) {

    }

    void draw(sf::RenderWindow& window) {
        window.draw(backgroundOverlay);


        static bool starsInit = false;
        static sf::CircleShape stars[150];
        if (!starsInit) {
            srand((unsigned int)time(nullptr) + 700);
            for (int i = 0; i < 150; i++) {
                float r = 1.0f + (rand() % 3);
                stars[i].setRadius(r);
                stars[i].setPosition((float)(rand() % 1920), (float)(rand() % 1080));
                int b = 150 + rand() % 100;
                stars[i].setFillColor(sf::Color((sf::Uint8)b, (sf::Uint8)b, (sf::Uint8)b));
            }
            starsInit = true;
        }
        for (int i = 0; i < 150; i++) {
            window.draw(stars[i]);
        }

        window.draw(titleText);
        for (int i = 0; i < 15; i++) {
            window.draw(creditsText[i]);
        }
        window.draw(backText);
    }
};



/// purpose: central controller managing the state flow (menu -> intro -> level1 -> level2 -> level3) and pausing.
/// parameters: keeps a single sfml render window and shares references with states to avoid copying heavy resources.
class Game {
private:
    sf::RenderWindow window;
    Menu menu;
    ShipSelection shipSelection;
    sf::Music backgroundMusic;
    IntroSequence intro;
    Level1 level1;
    Level1 level2;
    Level2Transition level2Transition;
    Level1 level3;
    Level2Transition level3Transition;
    VictoryStory victoryStory;
    TypewriterText level3StartText;
    sf::Font level3StartFont;
    sf::Clock level3StartTimer;
    TypewriterText level2StartText;
    sf::Font level2StartFont;
    sf::Clock level2StartTimer;
    PauseMenu pauseMenu;
    GameOver gameOverScreen;
    VictoryScreen victoryScreen;
    int state;
    bool isPaused;
    int totalScore;
    float totalTime;
    string selectedShipColor;

    HighScoreScreen highScoreScreen;
    bool scoreWasSaved;
    int selectedLevel;

    void drawLevel3() {
        level3.draw(window);
    }
    LevelSelection levelSelection;
    CreditsScreen creditsScreen;

public:

    Game()
        : window(sf::VideoMode(1920, 1080), "Galaxy Wars: The Ultimate Space Battle"),
        state(0),
        isPaused(false),
        totalScore(0),
        totalTime(0.0f),
        selectedShipColor("Red"),
        selectedLevel(0),
        scoreWasSaved(false)
    {
        window.setFramerateLimit(60);

        if (backgroundMusic.openFromFile("background_music.ogg")) {
            backgroundMusic.setLoop(true);
            backgroundMusic.setVolume(50.0f);
            backgroundMusic.play();
        }
        else {
            cout << "Warning: Could not load background music!" << endl;
        }

        if (!menu.loadFonts("Steelar-j9Vnj.ttf", "MaginerfreeRegular-ALodL.ttf")) {
            menu.loadFonts("arial.ttf", "arial.ttf");
        }

        menu.initialize(1920.0f, 1080.0f);
        if (!creditsScreen.loadFont("Steelar-j9Vnj.ttf")) {
            creditsScreen.loadFont("arial.ttf");
        }
        creditsScreen.initialize(1920.0f, 1080.0f);

        selectedLevel = 0;
        if (!levelSelection.loadFont("Steelar-j9Vnj.ttf")) {
            levelSelection.loadFont("arial.ttf");
        }
        levelSelection.initialize(1920.0f, 1080.0f);

        if (!shipSelection.loadFont("Steelar-j9Vnj.ttf")) {
            shipSelection.loadFont("arial.ttf");
        }
        shipSelection.loadShipTextures();
        shipSelection.initialize(1920.0f, 1080.0f);

        if (!intro.loadFont("MaginerfreeRegular-ALodL.ttf")) {
            intro.loadFont("arial.ttf");
        }

        intro.loadTypingSound("typing_sound.wav");
        level1.loadWaveFont("Steelar-j9Vnj.ttf");
        level2.loadWaveFont("Steelar-j9Vnj.ttf");

        level1.loadAssets();


        if (!level2Transition.loadFont("MaginerfreeRegular-ALodL.ttf")) {
            level2Transition.loadFont("arial.ttf");
        }
        level2Transition.loadSound("typing_sound.wav");


        level2.loadAssets("bg1.jpg");
        level2.configureDifficulty(0.8f, 0.8f, 540.0f, "Blue", 3);



        if (level2StartFont.loadFromFile("Steelar-j9Vnj.ttf")) {
            level2StartText.setup("LEVEL 2 STARTED", level2StartFont, 72, sf::Color::Cyan);
            level2StartText.setPosition(1920.0f * 0.5f, 1080.0f * 0.5f);
        }
        else if (level2StartFont.loadFromFile("arial.ttf")) {
            level2StartText.setup("LEVEL 2 STARTED", level2StartFont, 72, sf::Color::Cyan);
            level2StartText.setPosition(1920.0f * 0.5f, 1080.0f * 0.5f);
        }

        if (!level3Transition.loadFont("MaginerfreeRegular-ALodL.ttf")) {
            level3Transition.loadFont("arial.ttf");
        }
        level3Transition.loadSound("typing_sound.wav");


        level3.loadAssets("bg5.jpg", "Green");
        level3.configureDifficulty(0.6f, 0.8f, 600.0f, "Green", 3);
        level3.loadWaveFont("Steelar-j9Vnj.ttf");


        if (level3StartFont.loadFromFile("Steelar-j9Vnj.ttf")) {
            level3StartText.setup("LEVEL 3 - BOSS FIGHT", level3StartFont, 72, sf::Color::Red);
            level3StartText.setPosition(1920.0f * 0.5f, 1080.0f * 0.5f);
        }
        else if (level3StartFont.loadFromFile("arial.ttf")) {
            level3StartText.setup("LEVEL 3 - BOSS FIGHT", level3StartFont, 72, sf::Color::Red);
            level3StartText.setPosition(1920.0f * 0.5f, 1080.0f * 0.5f);
        }

        if (!victoryStory.loadFont("MaginerfreeRegular-ALodL.ttf")) {
            victoryStory.loadFont("arial.ttf");
        }
        victoryStory.loadTypingSound("typing_sound.wav");


        if (!pauseMenu.loadFont("MaginerfreeRegular-ALodL.ttf")) {
            pauseMenu.loadFont("arial.ttf");
        }
        pauseMenu.initialize(1920.0f, 1080.0f);


        if (!gameOverScreen.loadFont("MaginerfreeRegular-ALodL.ttf")) {
            gameOverScreen.loadFont("arial.ttf");
        }
        gameOverScreen.initialize(1920.0f, 1080.0f);


        if (!victoryScreen.loadFont("MaginerfreeRegular-ALodL.ttf")) {
            victoryScreen.loadFont("arial.ttf");
        }
        victoryScreen.initialize(1920.0f, 1080.0f);
        scoreWasSaved = false;



        if (!highScoreScreen.loadFont("MaginerfreeRegular-ALodL.ttf")) {
            highScoreScreen.loadFont("arial.ttf");
        }


    }

    /// purpose: main loop handling event polling, state updates, and draw order (background -> actors -> ui) per state.
    void run() {
        sf::Clock frameClock;

        while (window.isOpen()) {
            float dt = frameClock.restart().asSeconds();
            handleEvents();

            if (state == 0) {
                menu.update(dt);
            }
            else if (state == 1) {

                shipSelection.update(dt);
            }
            else if (state == 2) {
                intro.update();
                if (intro.isFinished()) {
                    level1.startTimer();
                    state = 3;
                }
            }
            else if (state == 3) {
                if (!isPaused) {
                    level1.update(dt);
                    int level1Target = level1.calculateTargetScore();
                    if (level1.getScore() >= level1Target && !level1.isPlayerDestroyed()) {
                        totalScore = level1.getScore();
                        totalTime = level1.getCurrentTime();
                        level1.stopTimer();

                        if (selectedLevel == 1) {

                            if (!scoreWasSaved) {
                                saveHighScore(currentPlayerName, totalScore, totalTime);
                                scoreWasSaved = true;
                            }
                            gameOverScreen.setVictory(true);
                            gameOverScreen.setScore(totalScore);
                            gameOverScreen.setTime(totalTime);
                            state = 4;
                        }
                        else {

                            level2Transition.start(1920.0f, 1080.0f);
                            state = 5;
                        }
                    }
                    else if (level1.isPlayerDestroyed()) {
                        totalScore = level1.getScore();
                        totalTime = level1.getCurrentTime();
                        level1.stopTimer();


                        if (!scoreWasSaved) {
                            saveHighScore(currentPlayerName, totalScore, totalTime);
                            scoreWasSaved = true;
                        }

                        gameOverScreen.setVictory(false);
                        gameOverScreen.setScore(totalScore);
                        gameOverScreen.setTime(totalTime);
                        state = 4;
                    }
                }
                else {
                    pauseMenu.update(dt);
                    int pauseAction = pauseMenu.handleInput(window);
                    if (pauseAction != PAUSE_NOTHING) {
                        handlePauseAction(pauseAction);
                    }
                }
            }

            else if (state == 4) {
                gameOverScreen.update(dt);
                int action = gameOverScreen.handleInput(window);
                if (action != PAUSE_NOTHING) {
                    handlePauseAction(action);
                }
            }
            else if (state == 5) {

                level2Transition.update();
                if (level2Transition.isFinished()) {

                    level2.reset();
                    level2.continueTimer(totalTime);
                    level2.setScoreOffset(totalScore);
                    level2StartText.start();
                    level2StartTimer.restart();
                    state = 7;
                }
            }
            else if (state == 7) {

                level2StartText.update(dt);
                if (level2StartTimer.getElapsedTime().asSeconds() >= 2.5f) {
                    state = 6;
                }
            }
            else if (state == 6) {
                if (!isPaused) {
                    level2.update(dt);
                    int level2Target = level2.calculateTargetScore();
                    if (level2.getScore() >= level2Target && !level2.isPlayerDestroyed()) {
                        totalScore = totalScore + level2.getScore();
                        totalTime = level2.getCurrentTime();
                        level2.stopTimer();

                        if (selectedLevel == 2) {

                            if (!scoreWasSaved) {
                                saveHighScore(currentPlayerName, totalScore, totalTime);
                                scoreWasSaved = true;
                            }
                            gameOverScreen.setVictory(true);
                            gameOverScreen.setScore(totalScore);
                            gameOverScreen.setTime(totalTime);
                            state = 4;
                        }
                        else {

                            level3Transition.start(1920.0f, 1080.0f);
                            state = 10;
                        }
                    }
                    else if (level2.isPlayerDestroyed()) {
                        totalScore = totalScore + level2.getScore();
                        totalTime = level2.getCurrentTime();
                        level2.stopTimer();

                        if (!scoreWasSaved) {
                            saveHighScore(currentPlayerName, totalScore, totalTime);
                            scoreWasSaved = true;
                        }

                        gameOverScreen.setVictory(false);
                        gameOverScreen.setScore(totalScore);
                        gameOverScreen.setTime(totalTime);
                        state = 4;
                    }
                }
                else {
                    pauseMenu.update(dt);
                    int pauseAction = pauseMenu.handleInput(window);
                    if (pauseAction != PAUSE_NOTHING) {
                        handlePauseAction(pauseAction);
                    }
                }
            }
            else if (state == 8) {

                victoryScreen.update(dt);
            }
            else if (state == 9) {
                highScoreScreen.draw(window);
            }
            else if (state == 10) {

                level3Transition.update();
                if (level3Transition.isFinished()) {
                    level3.reset();
                    level3.continueTimer(totalTime);
                    level3.setScoreOffset(totalScore);
                    level3StartText.start();
                    level3StartTimer.restart();
                    state = 12;
                }
            }
            else if (state == 12) {

                level3StartText.update(dt);
                if (level3StartTimer.getElapsedTime().asSeconds() >= 2.5f) {
                    state = 11;
                }
            }
            else if (state == 14) {
                levelSelection.update(dt);
                int levelChoice = levelSelection.handleInput(window);
                if (levelChoice > 0) {

                    sf::Font tempFont;
                    if (tempFont.loadFromFile("MaginerfreeRegular-ALodL.ttf") || tempFont.loadFromFile("arial.ttf")) {
                        currentPlayerName = getPlayerName(window, tempFont);
                    }
                    totalScore = 0;
                    totalTime = 0.0f;
                    scoreWasSaved = false;
                    selectedLevel = levelChoice;
                    state = 1;
                }
            }

            else if (state == 15) {
                creditsScreen.update(dt);
            }
            else if (state == 11) {

                if (!isPaused) {
                    level3.update(dt);


                    if (level3.isBossDefeated() && !level3.isPlayerDestroyed()) {
                        totalScore = totalScore + level3.getScore();
                        totalTime = level3.getCurrentTime();
                        level3.stopTimer();


                        victoryStory.start(1920.0f, 1080.0f);
                        state = 13;
                    }
                    else if (level3.isPlayerDestroyed()) {
                        totalScore = totalScore + level3.getScore();
                        totalTime = level3.getCurrentTime();
                        level3.stopTimer();

                        if (!scoreWasSaved) {
                            saveHighScore(currentPlayerName, totalScore, totalTime);
                            scoreWasSaved = true;
                        }

                        gameOverScreen.setVictory(false);
                        gameOverScreen.setScore(totalScore);
                        gameOverScreen.setTime(totalTime);
                        state = 4;
                    }
                }
                else {
                    pauseMenu.update(dt);
                    int pauseAction = pauseMenu.handleInput(window);
                    if (pauseAction != PAUSE_NOTHING) {
                        handlePauseAction(pauseAction);
                    }
                }
            }
            else if (state == 13) {

                victoryStory.update();
                if (victoryStory.isFinished()) {

                    if (!scoreWasSaved) {
                        saveHighScore(currentPlayerName, totalScore, totalTime);
                        scoreWasSaved = true;
                    }


                    gameOverScreen.setVictory(true);
                    gameOverScreen.setScore(totalScore);
                    gameOverScreen.setTime(totalTime);
                    state = 4;
                }
            }

            window.clear(sf::Color(5, 5, 25));

            if (state == 0) {
                menu.draw(window);
            }
            else if (state == 1) {
                shipSelection.draw(window);
            }
            else if (state == 2) {
                menu.draw(window);
                intro.draw(window);
            }
            else if (state == 3) {
                drawLevel1();
                if (isPaused) {
                    pauseMenu.draw(window);
                }
            }
            else if (state == 4) {
                drawLevel1();
                gameOverScreen.draw(window);
            }
            else if (state == 5) {

                drawLevel1();
                level2Transition.draw(window);
            }
            else if (state == 7) {

                drawLevel2();
                level2StartText.draw(window);
            }
            else if (state == 6) {

                drawLevel2();
                if (isPaused) {
                    pauseMenu.draw(window);
                }
            }
            else if (state == 8) {

                drawLevel2();
                victoryScreen.draw(window);
            }
            else if (state == 9) {

                highScoreScreen.draw(window);
            }
            else if (state == 10) {

                drawLevel2();
                level3Transition.draw(window);
            }
            else if (state == 12) {

                drawLevel3();
                level3StartText.draw(window);
            }
            else if (state == 13) {

                drawLevel3();
                victoryStory.draw(window);
            }
            else if (state == 14) {
                levelSelection.draw(window);
            }
            else if (state == 15) {
                creditsScreen.draw(window);
            }
            else if (state == 11) {

                drawLevel3();
                if (isPaused) {
                    pauseMenu.draw(window);
                }
            }

            window.display();
        }

        backgroundMusic.stop();
    }

private:
    /// purpose: process window events and route input based on current game state.

    void handleEvents() {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) {
                window.close();
            }

            if (e.type == sf::Event::KeyPressed) {
                if (state == 0) {
                    menu.handleInput(e.key.code);
                    if (e.key.code == sf::Keyboard::Enter) {
                        int sel = menu.getSelected();
                        if (sel == 0) {

                            sf::Font tempFont;
                            if (tempFont.loadFromFile("MaginerfreeRegular-ALodL.ttf") || tempFont.loadFromFile("arial.ttf")) {
                                currentPlayerName = getPlayerName(window, tempFont);
                            }
                            totalScore = 0;
                            totalTime = 0.0f;
                            scoreWasSaved = false;
                            selectedLevel = 0;
                            state = 1;
                        }

                        else if (sel == 1) {
                            highScoreScreen.loadScores();
                            highScoreScreen.initialize();
                            state = 9;
                        }
                        else if (sel == 2) {
                            state = 14;
                        }
                        else if (sel == 3) {
                            window.close();
                        }
                        else if (sel == 4) {
                            state = 15;
                        }
                    }
                }
                else if (state == 1) {

                    shipSelection.handleInput(e.key.code);
                    if (e.key.code == sf::Keyboard::Return) {
                        selectedShipColor = shipSelection.getSelectedShipColor();

                        if (selectedLevel == 1) {

                            level1.loadAssets("bg2.jpg", selectedShipColor);
                            level1.reset();
                            level1.setScoreOffset(0);
                            intro.start(1920.0f, 1080.0f);
                            state = 2;
                        }
                        else if (selectedLevel == 2) {

                            level2.loadAssets("bg1.jpg", selectedShipColor);
                            level2.reset();
                            level2.setScoreOffset(0);
                            level2.startTimer();
                            level2StartText.start();
                            level2StartTimer.restart();
                            state = 7;
                        }
                        else if (selectedLevel == 3) {

                            level3.loadAssets("bg5.jpg", selectedShipColor);
                            level3.reset();
                            level3.setScoreOffset(0);
                            level3.startTimer();
                            level3StartText.start();
                            level3StartTimer.restart();
                            state = 12;
                        }
                        else {

                            level1.loadAssets("bg2.jpg", selectedShipColor);
                            level1.reset();
                            level2.loadAssets("bg1.jpg", selectedShipColor);
                            level3.loadAssets("bg5.jpg", selectedShipColor);
                            intro.start(1920.0f, 1080.0f);
                            state = 2;
                        }
                    }
                }
                else if (state == 2) {

                    intro.skip();
                }
                else if (state == 3) {

                    if (e.key.code == sf::Keyboard::P && !level1.isPlayerDestroyed()) {
                        isPaused = !isPaused;
                    }

                    if (e.key.code == sf::Keyboard::Escape && !isPaused) {
                        menu.initialize(1920.0f, 1080.0f);
                        state = 0;
                    }

                    if (e.key.code == sf::Keyboard::R && level1.isPlayerDestroyed()) {
                        level1.reset();
                        intro.start(1920.0f, 1080.0f);
                        state = 2;
                    }
                }
                else if (state == 5) {

                    level2Transition.skip();
                }
                else if (state == 7) {

                    state = 6;
                }
                else if (state == 6) {

                    if (e.key.code == sf::Keyboard::P && !level2.isPlayerDestroyed()) {
                        isPaused = !isPaused;
                    }
                    if (e.key.code == sf::Keyboard::Escape && !isPaused) {
                        menu.initialize(1920.0f, 1080.0f);
                        state = 0;
                    }
                    if (e.key.code == sf::Keyboard::R && level2.isPlayerDestroyed()) {
                        level2.reset();
                        level2StartText.start();
                        level2StartTimer.restart();
                        state = 7;
                    }
                }
                else if (state == 8) {

                    if (e.key.code == sf::Keyboard::Return) {
                        totalScore = 0;
                        menu.initialize(1920.0f, 1080.0f);
                        state = 0;
                    }
                }
                else if (state == 9) {

                    if (e.key.code == sf::Keyboard::Escape) {
                        menu.initialize(1920.0f, 1080.0f);
                        state = 0;
                    }
                    else if (e.key.code == sf::Keyboard::Up || e.key.code == sf::Keyboard::Down) {
                        highScoreScreen.handleInput(e.key.code);
                    }
                }
                else if (state == 10) {

                    level3Transition.skip();
                }
                else if (state == 12) {

                    state = 11;
                }
                else if (state == 13) {

                    victoryStory.skip();
                }
                else if (state == 14) {
                    if (e.key.code == sf::Keyboard::Escape) {
                        menu.initialize(1920.0f, 1080.0f);
                        state = 0;
                    }
                }
                else if (state == 15) {
                    if (e.key.code == sf::Keyboard::Escape) {
                        menu.initialize(1920.0f, 1080.0f);
                        state = 0;
                    }
                }
                else if (state == 11) {

                    if (e.key.code == sf::Keyboard::P && !level3.isPlayerDestroyed()) {
                        isPaused = !isPaused;
                    }

                    if (e.key.code == sf::Keyboard::Escape && !isPaused) {
                        menu.initialize(1920.0f, 1080.0f);
                        state = 0;
                    }
                    if (e.key.code == sf::Keyboard::R && level3.isPlayerDestroyed()) {
                        level3.reset();
                        level3StartText.start();
                        level3StartTimer.restart();
                        state = 12;
                    }
                }
            }
        }
    }

    /// purpose: render level1 content to the shared render window.
    void drawLevel1() {
        level1.draw(window);
    }

    /// purpose: render level2 content to the shared render window.
    void drawLevel2() {
        level2.draw(window);
    }

    /// purpose: respond to pause menu selections and adjust state transitions accordingly.
    /// parameters: action identifies the selected pause option.
    void handlePauseAction(int action) {
        if (action == PAUSE_RESUME) {
            isPaused = false;
        }
        else if (action == PAUSE_RESTART) {
            isPaused = false;
            scoreWasSaved = false;
            totalScore = 0;
            totalTime = 0.0f;
            level1.reset();
            intro.start(1920.0f, 1080.0f);
            state = 1;
        }
        else if (action == PAUSE_MAIN_MENU) {
            isPaused = false;
            menu.initialize(1920.0f, 1080.0f);
            state = 0;
        }
        else if (action == PAUSE_EXIT) {
            window.close();
        }
    }
};


#endif
