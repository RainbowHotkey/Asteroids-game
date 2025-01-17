﻿#include <iostream>
#include <string>
#include <algorithm>
using namespace std;

#include "Game_engine/olcConsoleGameEngine.h"

class Hotkeys_Asteroids : public olcConsoleGameEngine
{
public:
    Hotkeys_Asteroids()
    {
        m_sAppName = L"Asteroids";
    }
    
private:
    struct SpaceObject
    {
        float x;
        float y;
        float dx;
        float dy;
        int nSize;
        float angle;
    };

    vector<SpaceObject> vecAsteroids;
    vector<SpaceObject> vecBullets;
    SpaceObject player;
    int nScore;
    bool Dead = false;

    vector<pair<float, float>> vecModelShip;
    vector<pair<float, float>> vecModelAsteroids;

protected:
    // called by olcConsoleGameEngine
    virtual bool OnUserCreate()
    {
        vecAsteroids.push_back({ 20.0f, 20.0f, 8.0f, -6.0f, (int)16 , 0.0f});

        // initialise player position
        player.x = ScreenWidth() / 2.0f;
        player.y = ScreenHeight() / 2.0f;
        player.dx = 0.0f;
        player.dy = 0.0f;
        player.angle = 0.0f;

        vecModelShip =
        {
            {0.0f, -5.0f},
            {-2.5f, +2.5f},
            {+2.5f, +2.5f}
        }; // simple isoceles triangle

        int verts = 20;
        for (int i = 0; i < verts; i++)
        {
            float radius = (float)rand() / (float)RAND_MAX * 0.4f + 0.8f;
            float a = ((float)i / (float)verts) * 6.28318f;
            vecModelAsteroids.push_back(make_pair(radius * sinf(a), radius * cosf(a)));
        }

        ResetGame();
        return true;
    }

    bool IsPointInsideCircle(float cx, float cy, float radius, float x, float y)
    {
        return sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy)) < radius;
    }

    void ResetGame()
    {
        vecAsteroids.clear();
        vecBullets.clear();

        vecAsteroids.push_back({ 20.0f, 20.0f, 8.0f, -6.0f, (int)16 , 0.0f });
        vecAsteroids.push_back({ 100.0f, 20.0f, -5.0f, -6.0f, (int)16 , 0.0f });

        // initialise player position
        player.x = ScreenWidth() / 2.0f;
        player.y = ScreenHeight() / 2.0f;
        player.dx = 0.0f;
        player.dy = 0.0f;
        player.angle = 0.0f;

        Dead = false;
        nScore = 0;
    }

    // called by olcConsoleGameEngine
    virtual bool OnUserUpdate(float fElapsedTime)
    {
        if (Dead)
            ResetGame();

        // clear screen
        Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, 0);


        // steer
        if (m_keys[VK_LEFT].bHeld)
            player.angle -= 5.0f * fElapsedTime;
        if (m_keys[VK_RIGHT].bHeld)
            player.angle += 5.0f * fElapsedTime;

        // Thrust
        if (m_keys[VK_UP].bHeld)
        {
            // acceleration changes velocity
            player.dx += sin(player.angle) * 20.0f * fElapsedTime;
            player.dy += -cos(player.angle) * 20.0f * fElapsedTime;
        }

        // velocity changes position
        player.x += player.dx * fElapsedTime;
        player.y += player.dy * fElapsedTime;

        // keep player in gamespace 
        WrapCoordinates(player.x, player.y, player.x, player.y);

        // ship collision wit asteroids
        for (auto& a : vecAsteroids)
            if (IsPointInsideCircle(a.x, a.y, a.nSize, player.x, player.y))
                Dead = true;


        if (m_keys[VK_SPACE].bReleased)
            vecBullets.push_back({ player.x, player.y, 50.0f * sinf(player.angle), -50.0f * cosf(player.angle) });

        // update and draw asteroids
        for (auto& a : vecAsteroids)
        {
            a.x += a.dx * fElapsedTime;
            a.y += a.dy * fElapsedTime;
            a.angle += 0.5f * fElapsedTime;
            WrapCoordinates(a.x, a.y, a.x, a.y);

            DrawWireFrameModel(vecModelAsteroids, a.x, a.y, a.angle, a.nSize, FG_YELLOW);
        }

        vector<SpaceObject> newAsteroids;

        // update and draw bullets
        for (auto& b : vecBullets)
        {
            b.x += b.dx * fElapsedTime;
            b.y += b.dy * fElapsedTime;
            WrapCoordinates(b.x, b.y, b.x, b.y);

            Draw(b.x, b.y);

            for (auto& a : vecAsteroids)
            {
                if (IsPointInsideCircle(a.x, a.y, a.nSize, b.x, b.y))
                {
                    // asteroid hit
                    b.x = -100;

                    if (a.nSize > 4)
                    {
                        // create two small asteroids
                        float angle1 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
                        float angle2 = ((float)rand() / (float)RAND_MAX) * 6.253185f;
                        newAsteroids.push_back({ a.x, a.y, 10.0f * sinf(angle1), 10.0f * cosf(angle1), (int)a.nSize >> 1, 0.0f });
                        newAsteroids.push_back({ a.x, a.y, 10.0f * sinf(angle2), 10.0f * cosf(angle2), (int)a.nSize >> 1, 0.0f });
                    }

                    a.x = -100;
                    nScore += 100;
                }
            }
        }

        for (auto a : newAsteroids)
            vecAsteroids.push_back(a);

        // remove off screen bullets
        if (vecBullets.size() > 0)
        {
            auto i = remove_if(vecBullets.begin(), vecBullets.end(),
                [&](SpaceObject o) {return (o.x < 1 | o.y < 1 || o.x >= ScreenWidth() - 1 || o.y >= ScreenHeight() - 1); });
            if (i != vecBullets.end())
                vecBullets.erase(i);
        }

        if (vecAsteroids.size() > 0)
        {
            auto i = remove_if(vecAsteroids.begin(), vecAsteroids.end(), 
                [&](SpaceObject o) {return (o.x < 0); });
            if (i != vecAsteroids.end())
                vecAsteroids.erase(i);
        }

        if (vecAsteroids.empty())
        {
            nScore += 1000;

            // add asteroids 90 degrees both sides of player
            vecAsteroids.push_back({ 30.0f * sinf(player.angle - 3.14159f / 2.0f),
                                    30.0f * cosf(player.angle - 3.14159f / 2.0f),
                                    10.0f * sinf(player.angle),
                                    10.0f * cosf(player.angle),
                                    (int)16, 0.0f });

            vecAsteroids.push_back({ 30.0f * sinf(player.angle + 3.14159f / 2.0f),
                                    30.0f * cosf(player.angle + 3.14159f / 2.0f),
                                    10.0f * sinf(-player.angle),
                                    10.0f * cosf(-player.angle),
                                    (int)16, 0.0f });
        }

        //Draw ship?
        DrawWireFrameModel(vecModelShip, player.x, player.y, player.angle, 2.0);

        // draw score
        DrawString(2, 2, L"SCORE: " + to_wstring(nScore));

        return true;
    }

    void DrawWireFrameModel(const vector<pair<float, float>>& vecModelCoordinates, float x, float y, float r = 0.0f, float s = 1.0f, short  col = FG_WHITE)
    {
        // pair.first = x coordinate
        // pair.second = y coordinate 
        
        //create translated model vector of coordinate pairs 
        vector<pair<float, float>> vecTransformedCoordinates;
        int verts = vecModelCoordinates.size();
        vecTransformedCoordinates.resize(verts);

        // rotate 
        for (int i = 0; i < verts; i++)
        {
            vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(r) - vecModelCoordinates[i].second * sinf(r);
            vecTransformedCoordinates[i].second = vecModelCoordinates[i].first * sinf(r) + vecModelCoordinates[i].second * cosf(r);
        }

        // scale 
        for (int i = 0; i < verts; i++)
        {
            vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * s;
            vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second * s;
        }

        // translate
        for (int i = 0; i < verts; i++)
        {
            vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first + x;
            vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second + y;
        }

        // draw closed polygon
        for (int i = 0; i < verts; i++)
        {
            int j = (i + 1);
            DrawLine(vecTransformedCoordinates[i % verts].first, vecTransformedCoordinates[i % verts].second,
                vecTransformedCoordinates[j % verts].first, vecTransformedCoordinates[j % verts].second, PIXEL_SOLID, col);
        }
    }



    void WrapCoordinates(float ix, float iy, float& ox, float& oy)
    {
        ox = ix;
        oy = iy;
        if (ix < 0.0f) ox = ix + (float)ScreenWidth();
        if (ix >= (float)ScreenWidth()) ox = ix - (float)ScreenWidth();
        if (iy < 0.0f) oy = iy + (float)ScreenHeight();
        if (iy >= (float)ScreenHeight()) oy = iy - (float)ScreenHeight();
    }

    virtual void Draw(int x, int y, wchar_t c = 0x2588, short col = 0x000F)
    {
        float fx, fy;
        WrapCoordinates(x, y, fx, fy);
        olcConsoleGameEngine::Draw(fx, fy, c, col);
    }
};

int main()
{  
    // use olcConsoleGameEngine derived app
    Hotkeys_Asteroids game;
    game.ConstructConsole(320, 200, 4, 4);
    game.Start();
    return 0;
}

