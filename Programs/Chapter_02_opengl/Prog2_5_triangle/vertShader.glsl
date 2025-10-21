// #version 430

// // 构建并返回平移矩阵
// mat4 buildTranslate(float x, float y, float z)
// { mat4 trans = mat4(1.0, 0.0, 0.0, 0.0, 
//                     0.0, 1.0, 0.0, 0.0, 
//                     0.0, 0.0, 1.0, 0.0, 
//                     x, y, z, 1.0 ); 
//   return trans;
// } 

// // 构建并返回绕x轴的旋转矩阵
// mat4 buildRotateX(float rad)
// { mat4 xrot = mat4(1.0, 0.0, 0.0, 0.0, 
//                    0.0, cos(rad), -sin(rad), 0.0, 
//                    0.0, sin(rad), cos(rad), 0.0, 
//                    0.0, 0.0, 0.0, 1.0 ); 
//   return xrot;
// }

// // 构建并返回绕y轴的旋转矩阵
// mat4 buildRotateY(float rad)
// { mat4 yrot = mat4(cos(rad), 0.0, sin(rad), 0.0, 
//                    0.0, 1.0, 0.0, 0.0, 
//                    -sin(rad), 0.0, cos(rad), 0.0, 
//                    0.0, 0.0, 0.0, 1.0 ); 
//   return yrot;
// }

// // 构建并返回绕z轴的旋转矩阵
// mat4 buildRotateZ(float rad)
// { mat4 zrot = mat4(cos(rad), -sin(rad), 0.0, 0.0, 
//                    sin(rad), cos(rad), 0.0, 0.0, 
//                    0.0, 0.0, 1.0, 0.0, 
//                    0.0, 0.0, 0.0, 1.0 ); 
//   return zrot;
// }

// // 构建并返回缩放矩阵
// mat4 buildScale(float x, float y, float z)
// { mat4 scale = mat4(x, 0.0, 0.0, 0.0, 
//                     0.0, y, 0.0, 0.0, 
//                     0.0, 0.0, z, 0.0, 
//                     0.0, 0.0, 0.0, 1.0 ); 
//   return scale;
// }

// uniform float angle;

// void main(void)
// { 
//   float rad = angle * 3.14159f / 180.0f;
//   mat4 temp = buildRotateZ(rad);
//   vec4 positions[3] = vec4[3](
//     vec4( 0.0, 0.0, 0.0, 1.0),
//     vec4( 0.0, 0.5, 0.0, 1.0), 
//     vec4( 0.5, 0.0, 0.0, 1.0)
//   );
//   gl_Position = temp * positions[gl_VertexID];
// }


#version 430

// 四元数乘法
vec4 quatMultiply(vec4 q1, vec4 q2)
{
    return vec4(
        q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
        q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
        q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
        q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
    );
}

// 从四元数构建旋转矩阵
mat4 quatToMatrix(vec4 q)
{
    float x = q.x, y = q.y, z = q.z, w = q.w;
    float x2 = x * x, y2 = y * y, z2 = z * z;
    float xy = x * y, xz = x * z, yz = y * z;
    float wx = w * x, wy = w * y, wz = w * z;
    
    return mat4(
        1.0f - 2.0f * (y2 + z2), 2.0f * (xy + wz), 2.0f * (xz - wy), 0.0f,
        2.0f * (xy - wz), 1.0f - 2.0f * (x2 + z2), 2.0f * (yz + wx), 0.0f,
        2.0f * (xz + wy), 2.0f * (yz - wx), 1.0f - 2.0f * (x2 + y2), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

// 从轴角创建四元数
vec4 axisAngleToQuat(vec3 axis, float angle)
{
    float halfAngle = angle * 0.5f;
    float s = sin(halfAngle);
    return vec4(axis * s, cos(halfAngle));
}

uniform float angle;

void main(void)
{ 
    float rad = angle * 3.14159f / 180.0f;
    
    // 使用四元数创建绕Z轴的旋转
    vec4 rotationQuat = axisAngleToQuat(vec3(0.0, 0.0, 1.0), rad);
    mat4 rotationMatrix = quatToMatrix(rotationQuat);
    
    vec4 positions[3] = vec4[3](
        vec4( 0.0, 0.0, 0.0, 1.0),
        vec4( 0.0, 0.5, 0.0, 1.0), 
        vec4( 0.5, 0.0, 0.0, 1.0)
    );
    
    gl_Position = rotationMatrix * positions[gl_VertexID];
}