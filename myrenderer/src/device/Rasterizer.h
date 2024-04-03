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
        //�ӿ����� to bufferƫ��,buffer����ͼ������ϵ,�ȼ���canvas����(y���ϵ��µݼ�),���跭תy
        //�����ӿڸ߶�20����,�����귶Χ[0,19],��ת��19->0 0->19,��canvas_y = 20-1-vp_y
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
            if (pntVec.x() * sideVec.y() - pntVec.y() * sideVec.x() > 0) { //��˽��������
                flag++;
            }
        }
        if (flag == 3 || flag == 0) {
            return true;    //���ν��ͬ��ʱ, flagֻ��Ϊ3��0
        } else {
            return false;
        }
    }
    

    void rasterize_with_ssaa(const Point2i& pos,const Vec4(&screenCoords)[3],const shared_ptr<Shader>& shader) {
        // using 2x2 super-sampling anti-aliasing
        // �����е�Ϊ(x,y),��һ��������4��������λ�÷ֱ�Ϊ(x-0.25,y-0.25),....
        //procedure: bounding box -> rasterize -> depth testing -> fragment shader -> write buffer
        //��Ȳ���Ҳ������fragment shader����֮��
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
                //��Ȳ���
                //zֵ��screen spaceΪ׼, ����Ҫ��view spaceΪ׼
                double sub_z_interpolated = Interpolator::screenspace_interpolate(bar, { screenCoords[0].z(), screenCoords[1].z(), screenCoords[2].z() });
                // ע��:zֵ��Զ���Ĺ�ϵ����projection��viewport�任��ͬ����,viewport�任��,zֵԽСԽԶ,������תΪdepthֵ,Խ��ԽԶ, depth_buf��ʼֵΪ�����
                double depth = -sub_z_interpolated; // ԽСԽ�� -> Խ��Խ��


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
                    _frameBuffer[index][i] = fragColor; //������������Ż�������ɫ,�����������buffer����
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
        initZBuf.subunits.resize(_ssaa_multiple, infinity);  //ע�⣡��ȳ�ʼֵӦΪ����,�����������������С���ݶ�Z��Ĵ���ʽ����,���߽��п��ܣ�
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
