//  Created by Igor Kroitor on 29/12/15.

//-----------------------------------------------------------------------------
// Gilbert-Johnson-Keerthi (GJK) collision detection algorithm in 2D
// http://www.dyn4j.org/2010/04/gjk-gilbert-johnson-keerthi/
// http://mollyrocket.com/849
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Basic vector arithmetic operations

Vector2 v2_negate(Vector2 v) { v.x = -v.x; v.y = -v.y; return v; }
Vector2 v2_perpendicular(Vector2 v) { Vector2 p = { v.y, -v.x }; return p; }
float v2_squared_length(Vector2 v) { return v.x * v.x + v.y * v.y; }

//-----------------------------------------------------------------------------
// Triple product expansion is used to calculate perpendicular normal vectors 
// which kinda 'prefer' pointing towards the Origin in Minkowski space

Vector2 v2_triple_product(Vector2 a, Vector2 b, Vector2 c) {
    
    Vector2 r;
    
    float ac = a.x * c.x + a.y * c.y; // perform a.dot(c)
    float bc = b.x * c.x + b.y * c.y; // perform b.dot(c)
    
    // perform b * a.dot(c) - a * b.dot(c)
    r.x = b.x * ac - a.x * bc;
    r.y = b.y * ac - a.y * bc;
    return r;
}

//-----------------------------------------------------------------------------
// This is to compute average center (roughly). It might be different from
// Center of Gravity, especially for bodies with nonuniform density,
// but this is ok as initial direction of simplex search in GJK.

Vector2 average_point (const Vector2 * vertices, size_t count) {
    Vector2 avg = { 0.f, 0.f };
    for (size_t i = 0; i < count; i++) {
        avg.x += vertices[i].x;
        avg.y += vertices[i].y;
    }
    avg.x /= count;
    avg.y /= count;
    return avg;
}

//-----------------------------------------------------------------------------
// Get furthest vertex along a certain direction

size_t furthest_point_index (const Vector2 * vertices, size_t count, Vector2 d) {
    
    float maxProduct = v2_dot (d, vertices[0]);
    size_t index = 0;
    for (size_t i = 1; i < count; i++) {
        float product = v2_dot (d, vertices[i]);
        if (product > maxProduct) {
            maxProduct = product;
            index = i;
        }
    }
    return index;
}

//-----------------------------------------------------------------------------
// Minkowski sum support function for GJK

Vector2 support (const Vector2 * vertices1, size_t count1,
              const Vector2 * vertices2, size_t count2, Vector2 d) {

    // get furthest point of first body along an arbitrary direction
    size_t i = furthest_point_index (vertices1, count1, d);
    
    // get furthest point of second body along the opposite direction
    size_t j = furthest_point_index (vertices2, count2, v2_negate (d));

    // subtract (Minkowski sum) the two points to see if bodies 'overlap'
    return v2_sub (vertices1[i], vertices2[j]);
}

//-----------------------------------------------------------------------------
// The GJK yes/no test

int iter_count = 0;

bool gjk (const Vector2 * vertices1, size_t count1,
         const Vector2 * vertices2, size_t count2) {
    
    size_t index = 0; // index of current vertex of simplex
    Vector2 a, b, c, d, ao, ab, ac, abperp, acperp, simplex[3];
    
    Vector2 position1 = average_point (vertices1, count1); // not a CoG but
    Vector2 position2 = average_point (vertices2, count2); // it's ok for GJK )

    // initial direction from the center of 1st body to the center of 2nd body
    d = v2_sub (position1, position2);
    
    // if initial direction is zero – set it to any arbitrary axis (we choose X)
    if ((d.x == 0) && (d.y == 0))
        d.x = 1.f;
    
    // set the first support as initial point of the new simplex
    a = simplex[0] = support (vertices1, count1, vertices2, count2, d);
    
    if (v2_dot (a, d) <= 0)
        return false; // no collision
    
    d = v2_negate (a); // The next search direction is always towards the origin, so the next search direction is v2_negate(a)
    
    while (1) {
        iter_count++;
        
        a = simplex[++index] = support (vertices1, count1, vertices2, count2, d);
        
        if (v2_dot (a, d) <= 0)
            return 0; // no collision
        
        ao = v2_negate (a); // from point A to Origin is just negative A
        
        // simplex has 2 points (a line segment, not a triangle yet)
        if (index < 2) {
            b = simplex[0];
            ab = v2_sub (b, a); // from point A to B
            d = v2_triple_product (ab, ao, ab); // normal to AB towards Origin
            if (v2_squared_length (d) == 0)
                d = v2_perpendicular (ab);
            continue; // skip to next iteration
        }
        
        b = simplex[1];
        c = simplex[0];
        ab = v2_sub (b, a); // from point A to B
        ac = v2_sub (c, a); // from point A to C
        
        acperp = v2_triple_product (ab, ac, ac);
        
        if (v2_dot (acperp, ao) >= 0) {
            
            d = acperp; // new direction is normal to AC towards Origin
            
        } else {
            
            abperp = v2_triple_product (ac, ab, ab);
            
            if (v2_dot (abperp, ao) < 0)
                return true; // collision
            
            simplex[0] = simplex[1]; // swap first element (point C)

            d = abperp; // new direction is normal to AB towards Origin
        }
        
        simplex[1] = simplex[2]; // swap element in the middle (point B)
        --index;
    }
    
    return false;
}