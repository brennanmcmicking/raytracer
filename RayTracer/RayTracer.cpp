#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <math.h>

#include "RaytraceResult.h"
#include "Vec.h"
#include "Color.h"
#include "Sphere.h"
#include "Light.h"
#include "World.h"
#include "Intersection.h"
#include "invert.h"
#include "ppm.h"

#define NEAR_ZERO 0.0000000000003

#define TINY_STEP 0.1

using namespace std;


/*
* Performs matrix multiplication between a Vector "o" in R4 and a 4x4 matrix
*/
Vec mult(Vec o, double mat[4][4]) {
    Vec output;
    output.x = o.x * mat[0][0] + o.y * mat[0][1] + o.z * mat[0][2] + o.w * mat[0][3];
    output.y = o.x * mat[1][0] + o.y * mat[1][1] + o.z * mat[1][2] + o.w * mat[1][3];
    output.z = o.x * mat[2][0] + o.y * mat[2][1] + o.z * mat[2][2] + o.w * mat[2][3];
    output.w = o.x * mat[3][0] + o.y * mat[3][1] + o.z * mat[3][2] + o.w * mat[3][3];
    return output;
}

/*
* Transposes the given matrix, storing it in "out"
*/
void transpose(double mat[4][4], double out[4][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            out[i][j] = mat[j][i];
        }
    }
}

/*
* Returns the dot product of the to given vectors
* Ignores the w component
*/
double dot(Vec one, Vec two) {
    return one.x * two.x + one.y * two.y + one.z * two.z;
}

/*
* Returns the magnitude of the given vector
*/
double mag(Vec v) {
    return sqrt(dot(v, v));
}

/*
* Clamps any values in a color that are higher than 1.0 to 1.0
*/
Color clamp(Color c) {
    return Color(min(c.r, 1.0), min(c.g, 1.0), min(c.b, 1.0), min(c.a, 1.0));
}

/*
* Returns the normalized version of the given Vector
*/
Vec normalize(Vec v) {
    double m = mag(v);
    return v / m;
}

/*
* Returns the closest intersection of a ray starting at Point "o" and travelling along Vector "v" in World "w"
*/
Intersection closest_intersection(Vec o, Vec v, World *w) {
    double closest = DBL_MAX;
    Sphere* closestObject = nullptr;
    o.w = 1; // enforce w component
    v.w = 0;
    Vec closestPoint;
    Vec closestNormal;
    bool reverseNormal = false;

    for (int i = 0; i < w->numSpheres; i++) {
        // transform ray inverse to sphere's transform
        Vec oPrime = mult(o, w->spheres[i]->inverse);
        Vec vPrime = mult(v, w->spheres[i]->inverse);

        // check if the transformed ray intersects with a canonical sphere (sphere @ 0,0,0 w/ r = 1)
        // if so, calculate Th value
        // use this Th value to find the intersection point in world coordinates
        
        double A = dot(vPrime, vPrime); // |c|^2
        double B = dot(oPrime, vPrime); // S dot c
        double C = dot(oPrime, oPrime) - 1; // |S|^2 - 1

        double prod = B * B - A * C;

        // cout << "oPrime: " << oPrime.x << ", " << oPrime.y << ", " << oPrime.z << ", " << oPrime.w << " ... ";
        // cout << "A: " << A << ", B: " << B << ", C: " << C << ", prod: " << prod << endl;
        
        if (prod > -NEAR_ZERO) {
            double th;
            if (prod < NEAR_ZERO) {
                // one intersection
                th = -B / A;
            }
            else {
                // two intersections
                double th1 = -B / A + sqrt(prod) / A;
                double th2 = -B / A - sqrt(prod) / A;
                th = min(th1, th2);
                if ((th1 < 0 && th2 > 0) || (th1 > 0 && th2 < 0)) {
                    th = max(th1, th2);
                    // the ray came from inside of the sphere
                    reverseNormal = true;
                }
            }
            if (th < closest && th > 0) {
                closest = th;
                closestObject = w->spheres[i];

                Vec transposedNormal = oPrime + vPrime * th;
                if (reverseNormal) {
                    transposedNormal = transposedNormal * -1;
                }
                closestNormal = mult(transposedNormal, w->spheres[i]->inverse_transpose);
            }
            // cout << "oPrime: " << oPrime.x << ", " << oPrime.y << ", " << oPrime.z << ", " << oPrime.w << " ... ";
            // cout << "A: " << A << ", B: " << B << ", C: " << C << " prod: " << prod << " th: " << th << endl;
        }
    }

    closestPoint = o + (v * closest);

    IntersectionType type = NONE;

    if (closestObject != nullptr) {
        // cout << "th: " << closest << ", " << "point: " << p.x << ", " << p.y << ", " << p.z << endl;
        type = SPHERE;
    }

    return Intersection(closestPoint , closestNormal, closestObject, type);
}

/*
* Returns the colour produced by casting a ray from the given Vector "o" along 
* the direction of Vector "v" with the given depth and World "w"
* o: starting point of ray (aka origin)
* v: direction vector to travel along
*/
RaytraceResult raytrace(int depth, Vec o, Vec v, World *w) {
    if (depth <= 0) {
        // if depth is 0, return black
        return RaytraceResult(Color(0.0, 0.0, 0.0, 1.0), NONE);
    }
    Intersection intersection = closest_intersection(o, v, w);
    if(intersection.intersecter == SPHERE) {
        Sphere* s = intersection.sphere;
        Vec p = intersection.point;

        // variables used in ADS lighting equation
        Vec N = normalize(intersection.normal);
        Vec V = Vec() - normalize(intersection.point);

        // From assignment suggestion:  PIXEL_COLOR[c] = Ka * Ia[c] * O[c] + for each point light(p) { Kd * Ip[c] * (N dot L) * O[c] + Ks * Ip[c] * (R dot V)n } + Kr * (Color returned from reflection ray)
        // where O[c] = object color, Ia[c] = ambient color, Ip[c] = light color
        Color sphereColor = Color(s->r, s->g, s->b, 1.0);
        Color ambient = Color(w->ambient.r, w->ambient.g, w->ambient.b, 1.0) * s->Ka;
        Color localColor = sphereColor * ambient * s->Ka;
        Color localColr = Color();
        for (int i = 2; i == 2; i++) {
            // Get the light & its point
            Light *l = w->lights[i];
            Vec lightPoint = Vec(l->x, l->y, l->z, 1.0);

            // more variables used in ADS lighting equation
            Vec L = normalize(lightPoint - p);
            Vec R = N * 2 * dot(N, L) - L;

            // check to see if the ray from the sphere point to the light point has any intersections
            Intersection shadowIntersec = closest_intersection(p + L * TINY_STEP, L, w);
            if (shadowIntersec.intersecter == NONE || mag(shadowIntersec.point - p) > mag(lightPoint - p) + TINY_STEP) {
                Color lightColor = Color(l->r, l->g, l->b, 1.0);
                Color diffuse = lightColor * sphereColor * max(dot(L, N), 0.0) * s->Kd;
                Color specular = Color(0, 0, 0, 1.0);
                if (dot(R, V) > 0) {
                    specular = lightColor * pow(max(dot(R, V), 0.0), s->n) * s->Ks;
                }
                localColor = localColor + diffuse; // +specular;
            }
        }

        // from the slides
        Vec reflectedDirection = N * -2 * dot(N, v) + v;
        RaytraceResult reflected = raytrace(depth - 1, p + reflectedDirection * TINY_STEP, reflectedDirection, w);
        Color reflectedColor = Color(0.0, 0.0, 0.0, 0.0);
        if (reflected.intersecter != NONE) {
            reflectedColor = reflected.color;
            // cout << "reflectedColor: " << reflectedColor.r << ", " << reflectedColor.g << ", " << reflectedColor.b << endl;
        }
        Color outputColor = localColor + reflectedColor * s->Kr;
        return RaytraceResult(clamp(outputColor), SPHERE);
    }
    
    // if no intersecting spheres, return NONE (pixel will be filled with background color)
    return RaytraceResult(Color(), NONE);
}

// used as predicate for file parsing
bool repeating_spaces(char first, char second) {
    return (first == ' ') && (second == ' ');
}

int main (int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Please provide a filename with the scene definition.\n";
        return 1;
    }

    double near = 0, left = 0, right = 0, top = 0, bottom = 0;
    int nCols = 0, nRows = 0;
    World *world = new World();
    double Br = 0, Bg = 0, Bb = 0;
    double Ar = 0, Ag = 0, Ab = 0;
    string output_name = "";

    string line;
    ifstream input_file(argv[1]);
    while (getline(input_file, line)) {
        // format each line
        replace(line.begin(), line.end(), '\t', ' '); // replace all tabs with spaces
        string::iterator end = unique(line.begin(), line.end(), repeating_spaces);
        line.erase(end, line.end());
        string command = line.substr(0, line.find(" "));
        int cmdLen = command.length() + 1;
        cout << line << endl;

        if (command == "NEAR") {
            near = stod(line.substr(cmdLen, line.length() - cmdLen));
        }
        else if (command == "LEFT") {
            left = stod(line.substr(cmdLen, line.length() - cmdLen));
        }
        else if (command == "RIGHT") {
            right = stod(line.substr(cmdLen, line.length() - cmdLen));
        }
        else if (command == "BOTTOM") {
            bottom = stod(line.substr(cmdLen, line.length() - cmdLen));
        }
        else if (command == "TOP") {
            top = stod(line.substr(cmdLen, line.length() - cmdLen));
        }
        else if (command == "RES") {
            int endOfCommand = command.length() + 1;
            int endOfnCols = line.find(" ", endOfCommand);
            int endOfnRows = line.length();

            nCols = stoi(line.substr(endOfCommand, endOfnCols - endOfCommand));
            nRows = stoi(line.substr(endOfnCols, endOfnRows - endOfnCols));
        }
        else if (command == "SPHERE") {
            int endOfName = line.find(" ", cmdLen);
            int endOfX = line.find(" ", endOfName + 1);
            int endOfY = line.find(" ", endOfX + 1);
            int endOfZ = line.find(" ", endOfY + 1);
            int endOfSx = line.find(" ", endOfZ + 1);
            int endOfSy = line.find(" ", endOfSx + 1);
            int endOfSz = line.find(" ", endOfSy + 1);
            int endOfR = line.find(" ", endOfSz + 1);
            int endOfG = line.find(" ", endOfR + 1);
            int endOfB = line.find(" ", endOfG + 1);
            int endOfKa = line.find(" ", endOfB + 1);
            int endOfKd = line.find(" ", endOfKa + 1);
            int endOfKs = line.find(" ", endOfKd + 1);
            int endOfKr = line.find(" ", endOfKs + 1);
            int endOfN = line.length();

            Sphere* sphere = new Sphere();
            sphere->name = line.substr(cmdLen, endOfName - cmdLen);
            sphere->x = stod(line.substr(endOfName, endOfX - endOfName));
            sphere->y = stod(line.substr(endOfX, endOfY - endOfX));
            sphere->z = stod(line.substr(endOfY, endOfZ - endOfY));
            sphere->sx = stod(line.substr(endOfZ, endOfSx - endOfZ));
            sphere->sy = stod(line.substr(endOfSx, endOfSy - endOfSx));
            sphere->sz = stod(line.substr(endOfSy, endOfSz - endOfSy));
            sphere->r = stod(line.substr(endOfSz, endOfR - endOfSz));
            sphere->g = stod(line.substr(endOfR, endOfG - endOfR));
            sphere->b = stod(line.substr(endOfG, endOfB - endOfG));
            sphere->Ka = stod(line.substr(endOfB, endOfKa - endOfB));
            sphere->Kd = stod(line.substr(endOfKa, endOfKd - endOfKa));
            sphere->Ks = stod(line.substr(endOfKd, endOfKs - endOfKd));
            sphere->Kr = stod(line.substr(endOfKs, endOfKr - endOfKs));
            sphere->n = stod(line.substr(endOfKr, endOfN - endOfKr));
            // create transformation matrix for sphere
            sphere->transformation[0][0] = sphere->sx;
            sphere->transformation[0][3] = sphere->x;
            sphere->transformation[1][1] = sphere->sy;
            sphere->transformation[1][3] = sphere->y;
            sphere->transformation[2][2] = sphere->sz;
            sphere->transformation[2][3] = sphere->z;
            invert_matrix(sphere->transformation, sphere->inverse); // inverse transformation matrix
            transpose(sphere->inverse, sphere->inverse_transpose); // inverse transpose transformation matrix
            world->spheres[world->numSpheres] = sphere;
            world->numSpheres += 1;
        }
        else if (command == "LIGHT") {
            int endOfName = line.find(" ", cmdLen);
            int endOfX = line.find(" ", endOfName + 1);
            int endOfY = line.find(" ", endOfX + 1);
            int endOfZ = line.find(" ", endOfY + 1);
            int endOfR = line.find(" ", endOfZ + 1);
            int endOfG = line.find(" ", endOfR + 1);
            int endOfB = line.length();;

            Light* light = new Light();
            light->name = line.substr(cmdLen, endOfName - cmdLen);
            light->x = stod(line.substr(endOfName, endOfX - endOfName));
            light->y = stod(line.substr(endOfX, endOfY - endOfX));
            light->z = stod(line.substr(endOfY, endOfZ - endOfY));
            light->r = stod(line.substr(endOfZ, endOfR - endOfZ));
            light->g = stod(line.substr(endOfR, endOfG - endOfR));
            light->b = stod(line.substr(endOfG, endOfB - endOfG));
            world->lights[world->numLights] = light;
            world->numLights += 1;
        }
        else if (command == "BACK") {
            int endOfCommand = cmdLen;
            int endOfR = line.find(" ", endOfCommand);
            int endOfG = line.find(" ", endOfR + 1);
            int endOfB = line.length();
            Br = stod(line.substr(endOfCommand, endOfR - endOfCommand));
            Bg = stod(line.substr(endOfR + 1, endOfG - endOfR));
            Bb = stod(line.substr(endOfG + 1, endOfB - endOfG));
        }
        else if (command == "AMBIENT") {
            int endOfCommand = cmdLen;
            int endOfR = line.find(" ", endOfCommand);
            int endOfG = line.find(" ", endOfR + 1);
            int endOfB = line.length();
            Ar = stod(line.substr(endOfCommand, endOfR - endOfCommand));
            Ag = stod(line.substr(endOfR + 1, endOfG - endOfR));
            Ab = stod(line.substr(endOfG + 1, endOfB - endOfG));
        }
        else if (command == "OUTPUT") {
            output_name = line.substr(cmdLen, line.length() - cmdLen);
        }
    }

    world->background = Color(Br, Bg, Bb, 1.0);
    world->ambient = Color(Ar, Ag, Ab, 1.0);

    // output the scene parameters as they've been parsed
    cout << "near: " << near << endl;
    cout << "left: " << left << endl;
    cout << "right: " << right << endl;
    cout << "top: " << top << endl;
    cout << "bottom: " << bottom << endl;
    cout << "nCols: " << nCols << endl;
    cout << "nRows: " << nRows << endl;
    cout << "numSpheres: " << world->numSpheres << endl;
    for (int i = 0; i < world->numSpheres; i++) {
        cout << world->spheres[i]->name << " N value: " << world->spheres[i]->n;
        cout << " x y z: " << world->spheres[i]->x << " " << world->spheres[i]->y << " " << world->spheres[i]->z << endl;
    }
    cout << "numLights: " << world->numLights << endl;
    for (int i = 0; i < world->numLights; i++) {
        cout << world->lights[i]->name << ", ";
        cout << " x y z: " << world->lights[i]->x << " " << world->lights[i]->y << " " << world->lights[i]->z << endl;
    }
    cout << "Background R: " << Br << endl;
    cout << "Background G: " << Bg << endl;
    cout << "Background B: " << Bb << endl;
    cout << "Ambient R: " << Ar << endl;
    cout << "Ambient G: " << Ag << endl;
    cout << "Ambient B: " << Ab << endl;
    cout << "Output name: " << output_name << endl;

    unsigned char* pixels = (unsigned char*)malloc(nCols * nRows * 3);
    int pixel_pos = 0;

    for (int h = nRows - 1; h >= 0; h--) {
        for (int w = 0; w < nCols; w++) { 
            // for each pixel on the screen
            // find closest intersection of ray with an object (call raytrace function w/ direction)
            double uc = left + (right * 2 * w) / (nCols);
            double ur = bottom + (top * 2 * h) / (nRows);

            Vec p;
            p.x = uc;
            p.y = ur;
            p.z = -near;

            // cout << "pixel point: " << p.x << ", " << p.y << ", " << p.z << " ... ";

            Vec dir;
            dir.x = p.x;
            dir.y = p.y;
            dir.z = p.z;
            dir.w = 0;
            dir = normalize(dir);

            RaytraceResult result = raytrace(4, p, dir, world);
            Color c = result.color;
            if (result.intersecter == NONE) {
                c = world->background;
            }
            pixels[pixel_pos]       = (unsigned char)(c.r * 255);
            pixels[pixel_pos + 1]   = (unsigned char)(c.g * 255);
            pixels[pixel_pos + 2]   = (unsigned char)(c.b * 255);
            pixel_pos += 3;
            // cout << c.r << ", " << c.g << ", " << c.b << endl;
        }
    }

    save_imageP6(nCols, nRows, output_name.c_str() , pixels);

    free(pixels);

    return 0;
}