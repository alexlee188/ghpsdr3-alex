

__kernel void waterfall(__write_only image2d_t image,
                         const float regionx, const float regiony,
                         const float regionwidth, const float regionheight,
                         const int pixelwidth, const int pixelheight,
                         const int maxIterations,
                         __global __read_only float4 *colors)
{
    int xpixel = get_global_id(0);
    int ypixel = get_global_id(1);
    float xin = regionx + xpixel * regionwidth / pixelwidth;
    float yin = regiony + (pixelheight - 1 - ypixel) * regionheight / pixelheight;
    int iteration = 0;
    float x = 0;
    float y = 0;
    while (iteration < maxIterations) {
        float x2 = x * x;
        float y2 = y * y;
        if ((x2 + y2) > 4.0)
            break;
        float xtemp = x2 - y2 + xin;
        y = 2 * x * y + yin;
        x = xtemp;
        ++iteration;
    }
    int2 pos = (int2)(xpixel, ypixel);
    if (iteration < (maxIterations - 1)) {
        // Use the Normalized Iteration Count Algorithm
        // to compute an interpolation value between two
        // adjacent colors for a continuous tone image.
        // From: http://math.unipa.it/~grim/Jbarrallo.PDF
        const float loglogb = log(log(2.0f));
        const float invlog2 = 1.0f / log(2.0f);
        float v = (loglogb - log(log(sqrt(x * x + y * y)))) * invlog2;
        float4 color = mix(colors[iteration], colors[iteration + 1], v);
        write_imagef(image, pos, color);
    } else if (iteration < maxIterations) {
        write_imagef(image, pos, colors[iteration]);
    } else {
        write_imagef(image, pos, (float4)(0, 0, 0, 1));
    }
}
