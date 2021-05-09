#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define PI 3.14159f
#define MAX(a, b) a > b ? a : b
#define MIN(a, b) a > b ? b : a

struct Point
{
    Point()
    {
    }

    Point(olc::vf2d _position, float _directionAngle, float _rotationAngle) :
        position(_position), directionAngle(_directionAngle), rotationAngle(_rotationAngle)
    {
    }
    olc::vf2d position = { 0.0f, 0.0f };
    float directionAngle = 0.0f;
    float rotationAngle = 0.0f;
    bool withinSensoryRange = false;
    olc::Pixel color;
};

struct Triangle
{
    Triangle()
    {
    }

    Triangle(olc::vf2d _p1, olc::vf2d _p2, olc::vf2d _p3) :
        p1(_p1), p2(_p2), p3(_p3)
    {
    }

    olc::vf2d p1 = { 0.0f,   -7.0f };
    olc::vf2d p2 = { -5.0f,   5.0f };
    olc::vf2d p3 = { 5.0f,    5.0f };

    Triangle TranslateAndRotate(const float rotationAngle, olc::vf2d offset)
    {
        Triangle tri;
        tri.p1.x = cosf(rotationAngle) * p1.x - sinf(rotationAngle) * p1.y + offset.x;
        tri.p1.y = sinf(rotationAngle) * p1.x + cosf(rotationAngle) * p1.y + offset.y;
        tri.p2.x = cosf(rotationAngle) * p2.x - sinf(rotationAngle) * p2.y + offset.x;
        tri.p2.y = sinf(rotationAngle) * p2.x + cosf(rotationAngle) * p2.y + offset.y;
        tri.p3.x = cosf(rotationAngle) * p3.x - sinf(rotationAngle) * p3.y + offset.x;
        tri.p3.y = sinf(rotationAngle) * p3.x + cosf(rotationAngle) * p3.y + offset.y;
        return tri;
    }
};

class PlayGround : public olc::PixelGameEngine
{

public:

    PlayGround()
    {
        sAppName = "PlayGround";
    }

private:

    bool debug = true;

private:

    Triangle agent1;
    float rotationAngle1 = 0.0f;
    float sensoryRadius1 = 50.0f;
    float fov1 = PI;
    float agent1Speed = 120.0f;
    float directionPointDistance1 = 60.0f;
    olc::vf2d position1 = { 300.0f, 150.0f };

private:

    olc::Pixel offWhite = olc::Pixel(200, 200, 200);

private:

    float pointsSpeed = 10.0f;
    int nPoints = 1000;
    std::vector<std::unique_ptr<Point>> points;

private:

    float GetDistance(float x1, float y1, float x2, float y2)
    {
        return sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
    }

    float DirectionAngle(float rotationAngle)
    {
        return rotationAngle - (PI / 2.0f);
    }

private:

    bool OnUserCreate() override
    {
        for (int i = 0; i < nPoints; i++)
        {
            //4 random floats between 0 and 1 for initializing x, y and rotation angle and direction angle for point
            float rx = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            float ry = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            float rra = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            float rda = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            std::unique_ptr<Point> point = std::make_unique<Point>(olc::vf2d(rx * 600, ry * 300), rda * (PI * 2), rra * (PI * 2));
            points.push_back(std::move(point));
        }

        return true;
    }

    bool OnUserUpdate(float elapsedTime) override
    {

        //USER CONTROLS

        if (GetKey(olc::UP).bHeld)
        {
            position1.x += cosf(DirectionAngle(rotationAngle1)) * elapsedTime * agent1Speed;
            position1.y += sinf(DirectionAngle(rotationAngle1)) * elapsedTime * agent1Speed;
        }
        if (GetKey(olc::RIGHT).bHeld)
            rotationAngle1 += 3.0f * elapsedTime;
        if (GetKey(olc::LEFT).bHeld)
            rotationAngle1 -= 3.0f * elapsedTime;
        if (GetKey(olc::Q).bHeld)
            fov1 -= 3.0f * elapsedTime;
        if (GetKey(olc::W).bHeld)
            fov1 += 3.0f * elapsedTime;
        if (GetKey(olc::A).bHeld)
            sensoryRadius1 -= 50.0f * elapsedTime;
        if (GetKey(olc::S).bHeld)
            sensoryRadius1 += 50.0f * elapsedTime;
        if (GetKey(olc::D).bPressed)
            debug = !debug;

        fov1 = MAX(MIN(fov1, PI), 0);
        sensoryRadius1 = MAX(MIN(sensoryRadius1, 200), 0);

        //TRANSFORMATIONS FOR TRIANGLE

        Triangle transformedAgent1 = agent1.TranslateAndRotate(rotationAngle1, position1);

        //points that connects to the triangle to show the directiom vector
        olc::vf2d direction1;
        direction1.x = (cosf(DirectionAngle(rotationAngle1)) * directionPointDistance1) + position1.x;
        direction1.y = (sinf(DirectionAngle(rotationAngle1)) * directionPointDistance1) + position1.y;

        //these are the two field of view points one at angle + fov and other at angle - fov
        olc::vf2d fovPoints11;
        olc::vf2d fovPoints12;

        //calculating position based on the position of triangle, fov and the sensory range
        fovPoints11.x = (cosf(DirectionAngle(rotationAngle1 + fov1)) * sensoryRadius1) + position1.x;
        fovPoints11.y = (sinf(DirectionAngle(rotationAngle1 + fov1)) * sensoryRadius1) + position1.y;

        fovPoints12.x = (cosf(DirectionAngle(rotationAngle1 - fov1)) * sensoryRadius1) + position1.x;
        fovPoints12.y = (sinf(DirectionAngle(rotationAngle1 - fov1)) * sensoryRadius1) + position1.y;

        //COLLISION DETECTION

        //within the sensory radius
        for (auto& point : points)
        {
            float distance = GetDistance(point->position.x, point->position.y, position1.x, position1.y);
            if (distance < sensoryRadius1)
                point->withinSensoryRange = true;
            else
            {
                point->color = olc::BLACK;
                point->withinSensoryRange = false;
            }
        }

        //within the field of view
        for (auto& point : points)
        {
            if (point->withinSensoryRange)
            {
                olc::vf2d normalizedForwardVector = (direction1 - position1).norm();
                olc::vf2d normalizedPointCentreVector = (point->position - position1).norm();

                float dot = normalizedPointCentreVector.dot(normalizedForwardVector);

                if (dot >= cosf(fov1))
                    debug ? point->color = olc::RED : point->color = olc::WHITE;
                else
                    debug ? point->color = olc::GREEN : point->color = olc::BLACK;
            }
        }

        //RENDERING 

        Clear(olc::Pixel(52, 55, 54));

        if (debug)
        {
            //draw control instructions
            DrawString(2, 40, "This is a toy program made to demonstrate how collision\ndetection within "
                "a field of view works. Black flies represent the\npoints that are comletely out "
                "of range. In debug mode,\nGreen ones represent the ones that are within the sensory\nraidus. The "
                "ones in the sensory radius are tested to\nsee if they are in the field of view, and "
                "if they\nare,they appear red.\n\nWhen debug mode is off, white flies\nrepresent the flies that can "
                "be seen", offWhite);

            DrawString(2, 10,
                "Press up, right and left keys for movement.\n"
                "Press w to increase FOV and q to reduce it.\n"
                "Press s to increase sensory range and a to decrease it.", offWhite);
        }

        DrawString(2, 290, "Press d to toggle text and geometric debug data.", olc::Pixel(200, 250, 200));

        //display info 
        std::ostringstream fovValue;
        fovValue << "FOV: " << round(fov1 * 2.0f * (180 / PI)) << " degrees";
        DrawString(440, 280, fovValue.str(), offWhite);

        std::ostringstream sensoryRangeValue;
        sensoryRangeValue << "Sensory Range: " << round(sensoryRadius1);
        DrawString(440, 265, sensoryRangeValue.str(), offWhite);


        //transform (wobble while moving forward) and draw all the points
        for (auto& point : points)
        {
            point->rotationAngle += 0.05f;
            point->directionAngle -= 0.05f;
            point->position.x += cosf(point->directionAngle) * sinf(point->rotationAngle) * elapsedTime * pointsSpeed;
            point->position.y += sinf(point->directionAngle) * sinf(point->rotationAngle) * elapsedTime * pointsSpeed;

            if (point->rotationAngle > PI * 2)
                point->rotationAngle = 0;
            if (point->rotationAngle < 0)
                point->rotationAngle = PI * 2;
            if (point->directionAngle > PI * 2)
                point->directionAngle = 0;
            if (point->directionAngle < 0)
                point->directionAngle = PI * 2;

            if (point->position.x > 600)
                point->position.x = 0;
            if (point->position.x < 0)
                point->position.x = 600;
            if (point->position.y > 300)
                point->position.y = 0;
            if (point->position.y < 0)
                point->position.y = 300;

            Draw((int)point->position.x, (int)point->position.y, point->color);
        }


        if (debug)
        {
            //lines from centre of triangle to fov points
            DrawLine((int)position1.x, (int)position1.y, (int)fovPoints11.x, (int)fovPoints11.y, olc::RED);
            DrawLine((int)position1.x, (int)position1.y, (int)fovPoints12.x, (int)fovPoints12.y, olc::RED);
            //field of view points
            FillCircle((int)fovPoints11.x, (int)fovPoints11.y, 2, olc::RED);
            FillCircle((int)fovPoints12.x, (int)fovPoints12.y, 2, olc::RED);
            //color the points between the two fov points in red
            float tempAngle = DirectionAngle(rotationAngle1 + fov1);
            while (tempAngle > DirectionAngle(rotationAngle1 - fov1))
            {
                for (int i = 0; i < 3; i++)
                    Draw((int)(cosf(tempAngle) * (sensoryRadius1 + i)) + position1.x,
                        (int)(sinf(tempAngle) * (sensoryRadius1 + i)) + position1.y, olc::RED);
                tempAngle -= 0.01f;
            }
            //draw sensory radius
            DrawCircle((int)position1.x, (int)position1.y, sensoryRadius1, olc::GREEN);
            //the straingt line signifying direction
            DrawLine((int)position1.x, (int)position1.y, (int)direction1.x, (int)direction1.y, offWhite);
        }
        //Draw the main triangle body
        FillTriangle(
            (int)transformedAgent1.p1.x, (int)transformedAgent1.p1.y,
            (int)transformedAgent1.p2.x, (int)transformedAgent1.p2.y,
            (int)transformedAgent1.p3.x, (int)transformedAgent1.p3.y,
            offWhite);


        return true;
    }
};

int main()
{
    PlayGround playGround;
    if (playGround.Construct(600, 300, 2, 2))
        playGround.Start();
}
