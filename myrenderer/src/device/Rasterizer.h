#pragma once
#ifndef DEVICE_RASTERIZER_H
#define DEVICE_RASTERIZER_H
#include <common/global.h>
#include <device/shader/Shader.h>
#include <device/Canvas.h>

class Rasterizer {
private:
    struct BoundingBox {
        int lft;
        int rgt;
        int top;
        int btn;
    };
    
    int _ssaa_multiple;

    template<typename T>
    using Subunits = std::vector<T>;

    template<typename T>
    struct BufUnit {
        Subunits<T> subunits;
        T& operator[](size_t i) {
            return subunits[i];
        }
    };
    using FrameBuffer = std::vector<BufUnit<ColorN>>;
    using ZBuffer = std::vector<BufUnit<double>>;
    FrameBuffer _frameBuffer;
    ZBuffer _zBuffer;

    int _width;
    int _height;

    BoundingBox _getBoundingBox(const Vec4(&screen_coords)[3]) {
        std::array<double, 3> coordx, coordy;
        std::array<Vec4, 3> v = { screen_coords[0],screen_coords[1],screen_coords[2] };
        std::transform(v.begin(), v.end(), coordx.begin(), [](Vec4& vec) { return vec.x(); });
        std::transform(v.begin(), v.end(), coordy.begin(), [](Vec4& vec) { return vec.y(); });
        int lft = *std::min_element(coordx.begin(), coordx.end()); //float -> int
        int rgt = *std::max_element(coordx.begin(), coordx.end()) + 1; //float -> int
        int top = *std::max_element(coordy.begin(), coordy.end()) + 1; //float -> int
        int btn = *std::min_element(coordy.begin(), coordy.end()); //float -> int
        return { std::max(0,lft),std::min(rgt,_width-1),std::min(top,_height-1),std::max(0,btn) };
    }

    size_t _viewportCoord_to_bufferOffset(const Point2i vp) {
        //视口坐标 to buffer偏移,buffer按照图像坐标系,先计算canvas坐标(y从上到下递减),故需翻转y
        //假设视口高度20像素,则坐标范围[0,19],翻转后19->0 0->19,即canvas_y = 20-1-vp_y
        int canvas_y = (_height - 1 - vp.y());
        return size_t(_width) * size_t(canvas_y) + size_t(vp.x());
    }

    static bool _insideTriangle(const Point2& p, const Vec4(&v)[3]) {
        Point2 pntVec;
        Point2 vertexes[3];
        vertexes[0] << v[0].head(2);
        vertexes[1] << v[1].head(2);
        vertexes[2] << v[2].head(2);
        int flag = 0;
        for (int i = 0; i < 3; i++) {
            Vec2 sideVec = vertexes[(i + 1) % 3] - vertexes[i];
            pntVec = p - vertexes[i];
            if (pntVec.x() * sideVec.y() - pntVec.y() * sideVec.x() > 0) { //叉乘结果有正负
                flag++;
            }
        }
        if (flag == 3 || flag == 0) {
            return true;    //三次结果同向时, flag只能为3或0
        } else {
            return false;
        }
    }
    

    void rasterize_with_ssaa(const Point2i& pos,const Vec4(&screenCoords)[3],const shared_ptr<Shader>& shader) {
        // using 2x2 super-sampling anti-aliasing
        // 像素中点为(x,y),则一个像素中4个子像素位置分别为(x-0.25,y-0.25),....
        //procedure: bounding box -> rasterize -> depth testing -> fragment shader -> write buffer
        //深度测试也可以在fragment shader过程之后
        Point2 delta1(0.25, 0.25), delta2(0.25, -0.25);
        Point2 fpos(pos.x(), pos.y());
        Point2 subpixels[4] = { fpos - delta1, fpos + delta1 ,fpos + delta2,fpos - delta2 };
        ColorN fragColor(0, 0, 0);
        size_t index = _viewportCoord_to_bufferOffset(pos);
        for (int i = 0; i < 4; i++) {
            Point2 subpixel = subpixels[i]; //+ Point2(0.5,0.5);
            //debug
            /*
            Vec3 bar = Interpolator::computeBarycentric2D(subpixel, screenCoords);
            shader->processVarying(bar);
            static int cnt = 0;
            Point2 uv = shader->v2f.textureCoord;
            std::cout << cnt++ << "\n";
            std::cout << "pixel:" << subpixels[i][0] << " " << subpixels[i][1] << "\n"; 
            std::cout<< "u = " << uv[0] << " v = " << uv[1] << "\n";
            */

            //debug
            if (_insideTriangle(subpixel, screenCoords)) {
                // If so, use the following code to get the interpolated z value.
                Vec3 bar = Interpolator::computeBarycentric2D(subpixel, screenCoords);
                shader->processVarying(bar);
                //深度测试
                //z值以screen space为准, 不需要以view space为准
                double sub_z_interpolated = Interpolator::screenspace_interpolate(bar, { screenCoords[0].z(), screenCoords[1].z(), screenCoords[2].z() });
                // 注意:z值与远近的关系需由projection和viewport变换共同决定,viewport变换后,z值越小越远,经过反转为depth值,越大越远, depth_buf初始值为无穷大
                double depth = -sub_z_interpolated; // 越小越深 -> 越大越深


                /*
                static int cnt = 0;
                std::cout << cnt++ << "\n";
                std::cout << "pixel:" << subpixels[i][0] << " " << subpixels[i][1] << "\n";
                std::cout << " z = " << sub_z_interpolated << "\n";*/
                if (_zBuffer[index][i] > depth) {
                    //debug
                    /*
                    static int cnt = 0;
                    std::cout << cnt++ << "\n";
                    std::cout << "pixel:" << subpixels[i][0] << " " << subpixels[i][1] << "\n";*/
                    //debug
                    _zBuffer[index][i] = depth;
                    shader->shadeFragment(fragColor);
                    _frameBuffer[index][i] = fragColor; //仅当此种情况才会设置颜色,其余情况保持buffer不变
                    /*
                    std::cout <<"color["<<i<<"] = " <<_frameBuffer[index][i][0] * 255.0 << " "
                        << _frameBuffer[index][i][0] * 255.0<<" "<< _frameBuffer[index][i][0] * 255.0<<"\n";*/
                }
            }
        }
    }


public:

    void setFrameSize(int width, int height) {
        _width = width;
        _height = height;
        _frameBuffer.resize(size_t(_width) * size_t(_height));
        _zBuffer.resize(size_t(_width) * size_t(_height));
    }

    void setSSAAMultiple(int multiple) {
        _ssaa_multiple = multiple;
    }

    void clearBuffer() {
        assert(_ssaa_multiple > 0);
        BufUnit<ColorN> initFBuf;
        BufUnit<double> initZBuf;
        initFBuf.subunits.resize(_ssaa_multiple, ColorN(0, 0, 0));
        initZBuf.subunits.resize(_ssaa_multiple, infinity);  //注意！深度初始值应为最深,具体是无穷大还是无穷小根据对Z轴的处理方式而定,两者皆有可能！
        std::fill(_frameBuffer.begin(), _frameBuffer.end(), initFBuf);
        std::fill(_zBuffer.begin(), _zBuffer.end(), initZBuf);
    }

    //Screen space rasterization
    void rasterize_triangle(const Vec4(&screenCoords)[3], const shared_ptr<Shader>& shader){
        BoundingBox bbox = _getBoundingBox(screenCoords);
        for (int x = bbox.lft; x <= bbox.rgt; x++) {
            for (int y = bbox.btn; y <= bbox.top; y++) {
                rasterize_with_ssaa(Point2i(x, y), screenCoords, shader);
            }
        }
    }

    void drawOnCanvas(shared_ptr<Canvas>& canvas) {
        size_t len = _frameBuffer.size();
        for (size_t i = 0; i < len;i++) {
            BufUnit pixel = _frameBuffer[i];
            ColorN pixelColor(0, 0, 0);
            for (const ColorN& c : pixel.subunits) {
                pixelColor += c;
            }
            pixelColor /= double(_ssaa_multiple);
            canvas->drawOnePixel(pixelColor, i);
        }
    }
};

#endif // DEVICE_RASTERIZER_H
