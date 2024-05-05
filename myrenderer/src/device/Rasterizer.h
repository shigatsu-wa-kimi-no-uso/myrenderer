#pragma once
#ifndef DEVICE_RASTERIZER_H
#define DEVICE_RASTERIZER_H
#include <common/global.h>
#include <device/shader/Shader.h>
#include <device/Canvas.h>
#include <common/Buffer.h>
#include <omp.h>

class Rasterizer {
private:
    struct BoundingBox {
        int lft;
        int rgt;
        int top;
        int btn;
    };
    
    int _ssaa_multiple = 0;

    shared_ptr<FrameBuffer> _frameBuffer = make_shared<FrameBuffer>();
    shared_ptr<ZBuffer> _zBuffer = make_shared<ZBuffer>();

    shared_ptr<FrameBuffer> _frameBufferUsing = _frameBuffer;
    shared_ptr<ZBuffer> _zBufferUsing = _zBuffer;

    int _width = 0;
    int _height = 0;

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

    size_t _viewportCoord_to_bufferOffset(const Point2i& vp) {
        //视口坐标 to buffer偏移,buffer按照图像坐标系,先计算canvas坐标(y从上到下递减),故需翻转y
        //假设视口高度20像素,则坐标范围[0,19],翻转后19->0 0->19,即canvas_y = 20-1-vp_y
        int canvas_y = (_height - 1 - vp.y());
        return size_t(_width) * size_t(canvas_y) + size_t(vp.x());
    }

    size_t _viewportCoord_to_bufferOffset(const int vx,const int vy) {
        //视口坐标 to buffer偏移,buffer按照图像坐标系,先计算canvas坐标(y从上到下递减),故需翻转y
        //假设视口高度20像素,则坐标范围[0,19],翻转后19->0 0->19,即canvas_y = 20-1-vp_y
        int canvas_y = (_height - 1 - vy);
        return size_t(_width) * size_t(canvas_y) + size_t(vx);
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
    

    void rasterize_ssaa(const Point2i& pos,const Vec4(&screenCoords)[3],const shared_ptr<Shader>& shader) {
        // using super-sampling anti-aliasing
        // 像素中点为(x,y),如果倍率为x4,则一个像素中4个子像素位置分别为(x-0.25,y-0.25),....
        //procedure: bounding box -> rasterize -> depth testing -> fragment shader -> write buffer
        //深度测试也可以在fragment shader过程之后
        int samples_per_line = sqrt(_ssaa_multiple);
        Vec2 delta(1.0 / samples_per_line, 1.0 / samples_per_line);
        Point2 fpos(pos[0], pos[1]);
        Point2 subpixel_upperleft = fpos - Vec2(0.5, 0.5) + delta / 2.0; //SSAA采样左上角第一个子像素位置
       // size_t bufferIdx = _viewportCoord_to_bufferOffset(Point2i(pos[0],pos[1]));
        for (int i = 0; i < samples_per_line; i++) {
            for (int j = 0; j < samples_per_line; j++) {
                Point2 subpixel(subpixel_upperleft[0] + i * delta[0], subpixel_upperleft[1] + j * delta[1]); //+ Point2(0.5,0.5);
                rasterize(subpixel, screenCoords, shader,  pos,i* samples_per_line + j);
            }
        }
    }


    void rasterize(const Point2& subpixelPos, const Vec4(&screenCoords)[3], const shared_ptr<Shader>& shader,const Point2i& pixelPos, int subunitIdx) {
        if (_insideTriangle(subpixelPos, screenCoords)) {
            ZBuffer& zBuffer = *_zBufferUsing;
            FrameBuffer& frameBuffer = *_frameBufferUsing;
            Vec3 bar = shader->interpolator.computeBarycentric2D(subpixelPos, screenCoords);
            shader->processVarying(bar);
            //深度测试
            //z值以screen space为准, 不需要以view space为准
            double sub_z_interpolated = shader->interpolator.screenspace_interpolate(bar, { screenCoords[0].z(), screenCoords[1].z(), screenCoords[2].z() });
            // 注意:z值与远近的关系需由projection和viewport变换共同决定,viewport变换后,z值越小越远,经过反转为depth值,越大越远, depth_buf初始值为无穷大
            double depth = -sub_z_interpolated; // 越小越深 -> 越大越深
            //zbuffer一定是二维的,如果不用SSAA,则设第二维的长度为1
            //#pragma omp critical
            {
                if (zBuffer[pixelPos[1]][pixelPos[0]][subunitIdx] > depth) {
                    zBuffer[pixelPos[1]][pixelPos[0]][subunitIdx] = depth;
                    ColorN fragColor(0, 0, 0);
                    shader->shadeFragment(fragColor);
                    frameBuffer[pixelPos[1]][pixelPos[0]][subunitIdx] = fragColor; //仅当此种情况才会设置颜色,其余情况保持buffer不变
                }
            }
        }
    }
    

public:

    void useExternalBuffer(shared_ptr<FrameBuffer> frameBuffer) {
        _frameBufferUsing = frameBuffer;
    }

    void useExternalBuffer(shared_ptr<ZBuffer> zBuffer) {
        _zBufferUsing = zBuffer;
    }

    void useInternalBuffer() {
        _frameBufferUsing = _frameBuffer;
        _zBufferUsing = _zBuffer;
    }

    void setFrameSize(int width, int height) {
        _width = width;
        _height = height;
    }

    void setSSAAMultiple(int multiple) {
        _ssaa_multiple = multiple;
    }

    int getSSAAMultiple() {
        return _ssaa_multiple;
    }

    void clearBuffer() {
        assert(_ssaa_multiple > 0);
        assert(_width > 0 && _height > 0);
        BufUnit<ColorN> initFBuf;
        BufUnit<double> initZBuf;
        initFBuf.subunits.resize(_ssaa_multiple, ColorN(0, 0, 0));
        initZBuf.subunits.resize(_ssaa_multiple, infinity);  //注意！深度初始值应为最深,具体是无穷大还是无穷小根据对Z轴的处理方式而定,两者皆有可能！

        std::vector<BufUnit<ColorN>> eachLine_FBuf;
        std::vector<BufUnit<double>> eachLine_ZBuf;
        eachLine_FBuf.resize( _width, initFBuf);
        eachLine_ZBuf.resize(_width, initZBuf);
        _frameBufferUsing->resize(_height);
        _zBufferUsing->resize(_width);
        std::fill(_frameBufferUsing->begin(), _frameBufferUsing->end(), eachLine_FBuf);
        std::fill(_zBufferUsing->begin(), _zBufferUsing->end(),eachLine_ZBuf);
    }

    //Screen space rasterization
    void rasterize_triangle(const Vec4(&screenCoords)[3], const shared_ptr<Shader>& shader){
        BoundingBox bbox = _getBoundingBox(screenCoords);
    #pragma omp parallel for
        for (int x = bbox.lft; x <= bbox.rgt; x++) {
            for (int y = bbox.btn; y <= bbox.top; y++) {
                rasterize_ssaa(Point2i(x, y), screenCoords, shader);
            }
        }
    }


    void drawOnCanvas(shared_ptr<Canvas>& canvas) {
        assert(_ssaa_multiple > 0);
        assert(_width > 0 && _height > 0);
        const FrameBuffer& buffer = *_frameBufferUsing;
    #pragma omp parallel for
        for (int y = 0; y < _height; y++) {
            for (int x = 0; x < _width; x++) {
                BufUnit pixel = buffer[y][x];
                ColorN pixelColor(0, 0, 0);
                for (const ColorN& c : pixel.subunits) {
                    pixelColor += c;
                }
                pixelColor /= double(_ssaa_multiple);
                size_t offset = _viewportCoord_to_bufferOffset(x, y); //图像坐标系中y轴朝下
                canvas->drawOnePixel(pixelColor, offset );
            }
        }
    }
};

#endif // DEVICE_RASTERIZER_H
