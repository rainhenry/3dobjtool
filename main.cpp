/****************************************************************************

    程序名称：将Blender生成的OBJ文件转换为用于OpenGLES渲染的C文件
    程序设计：rainhenry
    程序版本：REV 0.2
    创建日期：20201031

    版本修订：
        REV 0.1      rainhenry     20201031    创建文档
        REV 0.2      rainhenry     20201110    增加对OBJ参数的控制
                                               可以分别控制顶点、UV坐标、法线等信息的生成

****************************************************************************/
//---------------------------------------------------------------------------
//  包含头文件
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

//  定义存放顶点数据的结构体
typedef struct
{
    float x;
    float y;
    float z;
}SVertex;

//  定义存放顶点数据的容器
std::vector<SVertex> VertexVec;

//  定义存放UV坐标的数据的结构体
typedef struct
{
    float u;
    float v;
}SUV;

//  定义存放UV坐标数据的容器
std::vector<SUV> UVVec;

//  定义法线数据结构体
typedef struct
{
    float x;
    float y;
    float z;
}SVertexNormal;

//  定义法线数据的容器
std::vector<SVertexNormal> VertexNormalVec;

//  定义平面描述数据结构体
//  为小于0的时候表示信息索引不可用，大于等于0的时候为有效索引
typedef struct
{
    int point_index1;
    int uv_index1;
    int vn_index1;

    int point_index2;
    int uv_index2;
    int vn_index2;

    int point_index3;
    int uv_index3;
    int vn_index3;
}SPlaneInfo;

//  定义平面描述数据容器
std::vector<SPlaneInfo> PlaneInfoVec;

//  OBJ内部对象名字
std::string InternalName;

//  解码调试开关
#define DEBUG_DECODE       0

//  生成数据的个数
//  1=仅仅生成 顶点信息
//  2=生成顶点信息 + UV坐标信息
//  3=生成顶点信息 + UV坐标信息 + 法线信息
unsigned int gen_level = 1;

//  得到当前字符串中有多少个指定的符号
int GetStringCountChar(std::string in_str, char ch)
{
    int re = 0;
    int len = in_str.size();
    int i=0;
    for(i=0;i<len;i++)
    {
        if(in_str.at(i) == ch)
        {
            re++;
        }
    }
    return re;
}

//  从一个字符串中删除回车和换行
std::string DeleteNR(std::string in_str)
{
    int len = in_str.size();
    int i=0;
    for(i=0;i<len;i++)
    {
        if((in_str.at(i) == '\r') || (in_str.at(i) == '\n'))
        {
            in_str.erase(in_str.begin() + i);
            i--;
            len--;
        }
    }
    return in_str;
}

//  从文件解码OBJ到内存数据
void DecodingOBJ(FILE* pfile)
{
    //  检测指针
    if(pfile == 0)  return;

    //  定义读取缓冲区
    char buff[2048];

    int line_cnt = 0;

    //  循环读取每一行
    char* pre = 0;
    do
    {
        //  读取一行
        pre = fgets(buff, sizeof(buff), pfile);

        //  当读取失败
        if(pre == 0) break;

        //  当为内部名字
        if((buff[0] == 'o') && (buff[1] == ' '))
        {
            //  获取内部名字
            std::string tmp_str = &buff[2];

            //  删除字符串内的回车或换行
            InternalName = DeleteNR(tmp_str);

            #if DEBUG_DECODE
            printf("Internal Name:%s\r\n", InternalName.c_str());
            #endif
        }
        //  当为顶点数据
        else if((buff[0] == 'v') && (buff[1] == ' '))
        {
            //  定义临时顶点数据
            SVertex tmp_v;
            tmp_v.x = 0.0f;
            tmp_v.y = 0.0f;
            tmp_v.z = 0.0f;

            //  获取数据
            sscanf(&buff[2], "%f %f %f", &tmp_v.x, &tmp_v.y, &tmp_v.z);

            //  保存数据
            VertexVec.insert(VertexVec.end(), tmp_v);

            #if DEBUG_DECODE
            printf("v:%f %f %f\r\n", tmp_v.x, tmp_v.y, tmp_v.z);
            #endif
        }
        //  当为UV数据
        else if((buff[0] == 'v') && (buff[1] == 't') && (buff[2] == ' ') && (gen_level >= 2))
        {
            //  定义临时UV数据
            SUV tmp_t;
            tmp_t.u = 0.0f;
            tmp_t.v = 0.0f;

            //  获取数据
            sscanf(&buff[3], "%f %f", &tmp_t.u, &tmp_t.v);

            //  格式处理
            //tmp_t.u = 1.0f - tmp_t.u;
            tmp_t.v = 1.0f - tmp_t.v;

            //  保存数据
            UVVec.insert(UVVec.end(), tmp_t);

            #if DEBUG_DECODE
            printf("vt:%f %f\r\n", tmp_t.u, tmp_t.v);
            #endif
        }
        //  当为法线数据
        else if((buff[0] == 'v') && (buff[1] == 'n') && (buff[2] == ' ') && (gen_level >= 3))
        {
            //  定义临时法线数据
            SVertexNormal tmp_vn;
            tmp_vn.x = 0.0f;
            tmp_vn.y = 0.0f;
            tmp_vn.z = 0.0f;

            //  获取数据
            sscanf(&buff[3], "%f %f %f", &tmp_vn.x, &tmp_vn.y, &tmp_vn.z);

            //  保存数据
            VertexNormalVec.insert(VertexNormalVec.end(), tmp_vn);

            #if DEBUG_DECODE
            printf("vn:%f %f %f\r\n", tmp_vn.x, tmp_vn.y, tmp_vn.z);
            #endif
        }
        //  当为平面数据
        else if((buff[0] == 'f') && (buff[1] == ' '))
        {
            line_cnt++;
 
            //  获取当前字符串中含有多少个/符号
            int ch_cnt = GetStringCountChar(&buff[2], '/');

            //  定义临时平面数据
            SPlaneInfo tmp_p;
            tmp_p.point_index1 = -1;
            tmp_p.uv_index1 = -1;
            tmp_p.vn_index1 = -1;
            tmp_p.point_index2 = -1;
            tmp_p.uv_index2 = -1;
            tmp_p.vn_index2 = -1;
            tmp_p.point_index3 = -1;
            tmp_p.uv_index3 = -1;
            tmp_p.vn_index3 = -1;

            //  根据数量不同，判断OBJ的格式
            //  此时仅仅含有顶点数据
            if(ch_cnt == (0*3))
            {
                //  获取数据
                sscanf(&buff[2], "%d %d %d", 
                       &tmp_p.point_index1, 
                       &tmp_p.point_index2, 
                       &tmp_p.point_index3
                      );

                //  计算成0基序的格式
                tmp_p.point_index1--;
                tmp_p.point_index2--;
                tmp_p.point_index3--;
            
                //  保存数据
                PlaneInfoVec.insert(PlaneInfoVec.end(), tmp_p);

                #if DEBUG_DECODE
                printf("f:%d %d %d\r\n", 
                       tmp_p.point_index1, 
                       tmp_p.point_index2, 
                       tmp_p.point_index3
                      );
                #endif
            }
            //  此时有 顶点数据 和 UV数据
            else if(ch_cnt == (1*3))
            {
                //  获取数据
                sscanf(&buff[2], "%d/%d %d/%d %d/%d", 
                       &tmp_p.point_index1, 
                       &tmp_p.uv_index1,
                       &tmp_p.point_index2, 
                       &tmp_p.uv_index2,
                       &tmp_p.point_index3,
                       &tmp_p.uv_index3
                      );
            
                //  计算成0基序的格式
                tmp_p.point_index1--;
                tmp_p.uv_index1--;
                tmp_p.point_index2--;
                tmp_p.uv_index2--;
                tmp_p.point_index3--;
                tmp_p.uv_index3--;

                //  保存数据
                PlaneInfoVec.insert(PlaneInfoVec.end(), tmp_p);

                #if DEBUG_DECODE
                printf("f:%d/%d %d/%d %d/%d\r\n", 
                       tmp_p.point_index1, 
                       tmp_p.uv_index1,
                       tmp_p.point_index2, 
                       tmp_p.uv_index2,
                       tmp_p.point_index3,
                       tmp_p.uv_index3
                      );
                #endif
            }
            //  此时有 顶点数据 和 UV数据 和 法线数据
            else if(ch_cnt == (2*3))
            {
                //  获取数据
                sscanf(&buff[2], "%d/%d/%d %d/%d/%d %d/%d/%d", 
                       &tmp_p.point_index1, 
                       &tmp_p.uv_index1,
                       &tmp_p.vn_index1,
                       &tmp_p.point_index2, 
                       &tmp_p.uv_index2,
                       &tmp_p.vn_index2,
                       &tmp_p.point_index3,
                       &tmp_p.uv_index3,
                       &tmp_p.vn_index3
                      );
            
                //  计算成0基序的格式
                tmp_p.point_index1--;
                tmp_p.uv_index1--;
                tmp_p.vn_index1--;
                tmp_p.point_index2--;
                tmp_p.uv_index2--;
                tmp_p.vn_index2--;
                tmp_p.point_index3--;
                tmp_p.uv_index3--;
                tmp_p.vn_index3--;

                //  保存数据
                PlaneInfoVec.insert(PlaneInfoVec.end(), tmp_p);

                #if DEBUG_DECODE
                printf("f:%d/%d/%d %d/%d/%d %d/%d/%d\r\n", 
                       tmp_p.point_index1, 
                       tmp_p.uv_index1,
                       tmp_p.vn_index1,
                       tmp_p.point_index2, 
                       tmp_p.uv_index2,
                       tmp_p.vn_index2,
                       tmp_p.point_index3,
                       tmp_p.uv_index3,
                       tmp_p.vn_index3
                      );
                #endif
            }
            //  不支持的格式 忽略
            else
            {
            }
        }

    }
    while(pre != 0);

    printf("line_cnt = %d\r\n", line_cnt);
}

//  从路径中获取文件名，含扩展名
std::string GetFileNameExFromPath(std::string in_str)
{
    //  定义返回字符串
    std::string re_str;

    //  遍历每个字符
    int len = in_str.size();
    int i = 0;
    for(i=0;i<len;i++)
    {
        //  当不为路径分割符号，插入输出字符串
        if((in_str.at(len-1-i) != '\\')&&(in_str.at(len-1-i) != '/'))
        {
            re_str.insert(re_str.begin(), in_str.at(len-1-i));
        }
        //  当为路径分割，直接跳出
        else
        {
            break;
        }
    }

    //  返回字符串
    return re_str;
}

//  从文件名中删除文件中的扩展名，输入必须仅仅是带扩展名的文件名，不能是带路径的
std::string GetFileNameNoExFormFileName(std::string in_str)
{
    //  定义返回字符串
    std::string re_str;

    //  完全赋值给返回
    re_str = in_str;

    //  统计其中含有多少点
    int dot_cnt = 0;
    int len = in_str.size();
    int i = 0;
    for(i=0;i<len;i++)
    {
        //  当为点的时候
        if(in_str.at(len-1-i) == '.')
        {
            dot_cnt++;
        }
    }

    //  只要里面含有点
    if(dot_cnt >= 1)
    {
        //  遍历每个字符，从后面网前执行删除
        for(i=0;i<len;i++)
        {
            //  当为点
            if(re_str.at(len-1-i) == '.')
            {
                //  删除
                re_str.erase(re_str.end()-1);
                break;
            }
            //  不为点
            else
            {
                re_str.erase(re_str.end()-1);
            }
        }
    }

    //  返回字符串
    return re_str;
}

//  从完整路径或文件名中提取纯文件名部分，不含扩展名
std::string GetOnlyFileNameNoEx(std::string in_str)
{
    return GetFileNameNoExFormFileName(GetFileNameExFromPath(in_str));
}

//  从完整路径含文件名中提取出纯路径部分
std::string GetOnlyFilePath(std::string in_str)
{
    //  定义返回字符串
    std::string re_str;

    //  完全赋值给返回
    re_str = in_str;

    //  统计其中含有多少个路径分割符号
    int ch_cnt = 0;
    int len = in_str.size();
    int i = 0;
    for(i=0;i<len;i++)
    {
        //  当为路径分割符号的时候
        if((in_str.at(len-1-i) == '\\') || (in_str.at(len-1-i) == '/'))
        {
            ch_cnt++;
        }
    }

    //  只要里面含有路径分割符号
    if(ch_cnt >= 1)
    {
        //  遍历每个字符，从后面网前执行删除
        for(i=0;i<len;i++)
        {
            //  当为分割符号
            if((in_str.at(len-1-i) == '\\') || (in_str.at(len-1-i) == '/'))
            {
                //  删除
                re_str.erase(re_str.end()-1);
                break;
            }
            //  不为分割符号
            else
            {
                re_str.erase(re_str.end()-1);
            }
        }
    }
    //  当没有分隔符的时候，返回空
    else
    {
        re_str.clear();
    }

    //  返回字符串
    return re_str;
}

//  根据内存中的数据生成对应的C程序,成功返回0
int GenCCode(std::string in_filename)
{
    //  生成目标文件的完全路径
    std::string filename;
    filename += GetOnlyFilePath(in_filename);
    filename += GetOnlyFileNameNoEx(in_filename);
    filename += ".c";

    //  尝试创建新文件
    FILE* pfile_c = fopen(filename.c_str(), "w");

    //  检测文件是否创建成功
    if(pfile_c == 0)  return -1;

    //  定义临时字符串变量
    std::string tmp_str;

    //  生成包含头文件
    //  #include "cube.h"
    tmp_str = "#include \"";
    tmp_str += GetOnlyFileNameNoEx(in_filename);
    tmp_str += ".h\"\r\n";
    fprintf(pfile_c, "%s", tmp_str.c_str());    //  写入文件

    //  计算数据总量，单位float个
    unsigned int dot_float = 0;           //  一个点有多少个float组成
    dot_float = 3;                        //  最少的时候，1个点有3个坐标xyz组成
    if(UVVec.size() > 0) dot_float += 2;  //  当存在UV贴图信息时，还需要两个float表示uv坐标
    if(VertexNormalVec.size() > 0) dot_float += 3;  //  当存在法线信息时，存在法线向量
    unsigned int float_cnt = dot_float * PlaneInfoVec.size() * 3;  //  每个平面有3个点确定

    //  生成数据头部
    //  const float cube_3d_vtn_data[324852354] = 
    //  {
    tmp_str = "const float ";
    tmp_str += GetOnlyFileNameNoEx(in_filename);
    tmp_str += "_3d_v";
    if(UVVec.size() > 0) tmp_str += "t";
    if(VertexNormalVec.size() > 0) tmp_str += "n";
    tmp_str += "_data[";
    char num_str[200];
    memset(num_str, 0, sizeof(num_str));
    sprintf(num_str, "%d", float_cnt);
    tmp_str += num_str;
    tmp_str += "] =\r\n{\r\n";
    fprintf(pfile_c, "%s", tmp_str.c_str());    //  写入文件

    //  开始写入数据
    int plane_cnt=0;
    int total_plane = PlaneInfoVec.size();    //  获取可用平面数量
    int total_vex = VertexVec.size();         //  获取可用顶点数量
    int total_uv = UVVec.size();              //  获取可用UV数量
    int total_vn = VertexNormalVec.size();    //  获取可用法线数量
    for(plane_cnt=0;plane_cnt<total_plane;plane_cnt++)   //  遍历每个平面
    {
        //  定义临时变量
        SPlaneInfo tmp_info = PlaneInfoVec.at(plane_cnt);   //  获取当前平面信息
        SVertex tmp_v;                                      //  临时顶点数据

        //-------------------------------------------------写入顶点数据1
        //  检查平面序号
        if((tmp_info.point_index1 >= total_vex) || (tmp_info.point_index1 < 0)) 
        {
            fclose(pfile_c);    //  关闭文件 释放资源
            return -2;
        }

        //  获取顶点数据
        tmp_v = VertexVec.at(tmp_info.point_index1);

        //  写入顶点数据
        fprintf(pfile_c, "    %f, %f, %f,    ", tmp_v.x, tmp_v.y, tmp_v.z);

        //  检查是否含有UV数据
        if((tmp_info.uv_index1 < total_uv) && (tmp_info.uv_index1 >= 0)) 
        {
            //  定义临时UV数据
            SUV tmp_uv;

            //  获取UV数据
            tmp_uv = UVVec.at(tmp_info.uv_index1);

            //  写入UV数据
            fprintf(pfile_c, "%f, %f,    ", tmp_uv.u, tmp_uv.v);
        }

        //  检查是否含有法线数据
        if((tmp_info.vn_index1 < total_vn) && (tmp_info.vn_index1 >= 0) && (gen_level >= 2)) 
        {
            //  定义临时法线数据
            SVertexNormal tmp_vn;

            //  获取法线数据
            tmp_vn = VertexNormalVec.at(tmp_info.vn_index1);

            //  写入法线数据
            fprintf(pfile_c, "%f, %f, %f,    ", tmp_vn.x, tmp_vn.y, tmp_vn.z);
        }

        //  完成一个点的写入
        fprintf(pfile_c, "\r\n");

        //-------------------------------------------------写入顶点数据2
        //  检查平面序号
        if((tmp_info.point_index2 >= total_vex) || (tmp_info.point_index2 < 0) && (gen_level >= 3)) 
        {
            fclose(pfile_c);    //  关闭文件 释放资源
            return -2;
        }

        //  获取顶点数据
        tmp_v = VertexVec.at(tmp_info.point_index2);

        //  写入顶点数据
        fprintf(pfile_c, "    %f, %f, %f,    ", tmp_v.x, tmp_v.y, tmp_v.z);

        //  检查是否含有UV数据
        if((tmp_info.uv_index2 < total_uv) && (tmp_info.uv_index2 >= 0)) 
        {
            //  定义临时UV数据
            SUV tmp_uv;

            //  获取UV数据
            tmp_uv = UVVec.at(tmp_info.uv_index2);

            //  写入UV数据
            fprintf(pfile_c, "%f, %f,    ", tmp_uv.u, tmp_uv.v);
        }

        //  检查是否含有法线数据
        if((tmp_info.vn_index2 < total_vn) && (tmp_info.vn_index2 >= 0)) 
        {
            //  定义临时法线数据
            SVertexNormal tmp_vn;

            //  获取法线数据
            tmp_vn = VertexNormalVec.at(tmp_info.vn_index2);

            //  写入法线数据
            fprintf(pfile_c, "%f, %f, %f,    ", tmp_vn.x, tmp_vn.y, tmp_vn.z);
        }

        //  完成一个点的写入
        fprintf(pfile_c, "\r\n");
        
        //-------------------------------------------------写入顶点数据3
        //  检查平面序号
        if((tmp_info.point_index3 >= total_vex) || (tmp_info.point_index3 < 0)) 
        {
            fclose(pfile_c);    //  关闭文件 释放资源
            return -2;
        }

        //  获取顶点数据
        tmp_v = VertexVec.at(tmp_info.point_index3);

        //  写入顶点数据
        fprintf(pfile_c, "    %f, %f, %f,    ", tmp_v.x, tmp_v.y, tmp_v.z);

        //  检查是否含有UV数据
        if((tmp_info.uv_index3 < total_uv) && (tmp_info.uv_index3 >= 0)) 
        {
            //  定义临时UV数据
            SUV tmp_uv;

            //  获取UV数据
            tmp_uv = UVVec.at(tmp_info.uv_index3);

            //  写入UV数据
            fprintf(pfile_c, "%f, %f,    ", tmp_uv.u, tmp_uv.v);
        }

        //  检查是否含有法线数据
        if((tmp_info.vn_index3 < total_vn) && (tmp_info.vn_index3 >= 0)) 
        {
            //  定义临时法线数据
            SVertexNormal tmp_vn;

            //  获取法线数据
            tmp_vn = VertexNormalVec.at(tmp_info.vn_index3);

            //  写入法线数据
            fprintf(pfile_c, "%f, %f, %f,    ", tmp_vn.x, tmp_vn.y, tmp_vn.z);
        }

        //  完成一个面的写入
        fprintf(pfile_c, "\r\n\r\n");
    }

    //  结束
    //  };
    tmp_str = "};\r\n";
    fprintf(pfile_c, "%s", tmp_str.c_str());    //  写入文件

    //  关闭文件
    fclose(pfile_c);

    //  生成目标文件的完全路径
    filename = "";
    filename += GetOnlyFilePath(in_filename);
    filename += GetOnlyFileNameNoEx(in_filename);
    filename += ".h";

    //  创建头文件
    FILE* pfile_h = fopen(filename.c_str(), "w");

    //  检测文件是否创建成功
    if(pfile_h == 0)  return -1;

    //  生成包含头文件
    //  #ifndef __cube_h__
    //  #define __cube_h__
    tmp_str = "#ifndef __";
    tmp_str += GetOnlyFileNameNoEx(in_filename);
    tmp_str += "_h__\r\n";
    tmp_str += "#define __";
    tmp_str += GetOnlyFileNameNoEx(in_filename);
    tmp_str += "_h__\r\n";
    fprintf(pfile_h, "%s", tmp_str.c_str());    //  写入文件

    //  生成C++/C兼容
    tmp_str = "#ifdef __cplusplus\r\nextern \"C\"\r\n{\r\n#endif\r\n";
    fprintf(pfile_h, "%s", tmp_str.c_str());    //  写入文件

    //  生成数据头部
    //  extern const float cube_3d_vtn_data[324852354];
    tmp_str = "extern const float ";
    tmp_str += GetOnlyFileNameNoEx(in_filename);
    tmp_str += "_3d_v";
    if(UVVec.size() > 0) tmp_str += "t";
    if(VertexNormalVec.size() > 0) tmp_str += "n";
    tmp_str += "_data[";
    memset(num_str, 0, sizeof(num_str));
    sprintf(num_str, "%d", float_cnt);
    tmp_str += num_str;
    tmp_str += "];\r\n";
    fprintf(pfile_h, "%s", tmp_str.c_str());    //  写入文件

    //  生成C++/C兼容
    tmp_str = "#ifdef __cplusplus\r\n}\r\n#endif\r\n";
    fprintf(pfile_h, "%s", tmp_str.c_str());    //  写入文件

    //  结束
    //  #endif
    tmp_str = "#endif \r\n";
    fprintf(pfile_h, "%s", tmp_str.c_str());    //  写入文件
    
    //  关闭文件
    fclose(pfile_h);

    //  操作成功
    return 0;
}

//---------------------------------------------------------------------------
//  主函数
int main(int argc, char** argv)
{
    //  打印信息
    printf("\r\n");
    printf("--------------3D OBJ to C Tool----------------\r\n");
    printf("--------------REV 0.2 20201110----------------\r\n");
    printf("----------------By rainhenry------------------\r\n");

    //  检查输入参数的个数
    //  当参数个数错误
    if(argc != 3)
    {
        printf("Input arg number Error!!\r\n");
        return -1;
    }

    //  尝试打开obj文件
    FILE* pfile_obj = fopen(argv[2], "r");
    if(pfile_obj == 0)
    {
        printf("File Open Error!!\r\n");
        return -2;
    }

    //  获取生成等级
    sscanf(argv[1], "%d", &gen_level);
    if((gen_level != 1) && (gen_level != 2) && (gen_level != 3))
    {
        printf("Not Support Generate Level!!\r\n");
        return -3;
    }
    printf("Generate Level = %d\r\n", gen_level);

    //  获取输入文件的纯名字部分，不含扩展名
    std::string filename_only_str = GetOnlyFileNameNoEx(argv[2]);
    printf("Input File Name:%s\r\nOBJ Name:%s\r\n",
           argv[2],
           filename_only_str.c_str()
          );

    //  解码该文件
    DecodingOBJ(pfile_obj);

    //  写入到C文件和H文件
    int re = GenCCode(argv[2]);

    //  当生成失败
    if(re != 0)
    {
        printf("Gen C Code Error!!\r\n");

        //  关闭obj文件
        fclose(pfile_obj);

        //  返回失败
        return -3;
    }
    //  生成成功
    else
    {
        printf("Gen %d Plane!!\r\n", (int)PlaneInfoVec.size());
    }

    //  关闭obj文件
    fclose(pfile_obj);

    //  返回成功
    return 0;
}


//---------------------------------------------------------------------------
//  文件结束


