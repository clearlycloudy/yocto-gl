//
// LICENSE:
//
// Copyright (c) 2016 -- 2017 Fabio Pellacini
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

// general includes ------------
#include <map>
#include <set>

#include "../yocto/yocto_gltf.h"
#include "../yocto/yocto_img.h"
#include "../yocto/yocto_math.h"
#include "../yocto/yocto_obj.h"
#include "../yocto/yocto_utils.h"

#include "ext/ArHosekSkyModel.h"

//
// Make standard shape. Public API described above.
//
void make_uvhollowcutsphere(int usteps, int vsteps, float radius,
    std::vector<ym::vec3i>& triangles, std::vector<ym::vec3f>& pos,
    std::vector<ym::vec3f>& norm, std::vector<ym::vec2f>& texcoord) {
    triangles.clear();
    pos.clear();
    norm.clear();
    texcoord.clear();

    std::vector<ym::vec3f> mpos, mnorm;
    std::vector<ym::vec2f> mtexcoord;
    std::vector<ym::vec3i> mtriangles;
    make_uvcutsphere(
        usteps, vsteps, radius, mtriangles, mpos, mnorm, mtexcoord);
    for (auto& uv : mtexcoord) uv.y *= radius;
    ym::merge_triangles(
        triangles, pos, norm, texcoord, mtriangles, mpos, mnorm, mtexcoord);
    make_uvflippedcutsphere(
        usteps, vsteps, radius, mtriangles, mpos, mnorm, mtexcoord);
    for (auto& p : mpos) p *= radius;
    ym::merge_triangles(
        triangles, pos, norm, texcoord, mtriangles, mpos, mnorm, mtexcoord);
    // dpdu = [- s r s0 s1, s r c0 s1, 0] === [- s0, c0, 0]
    // dpdv = [s c0 s1, s s0 s1, s c1] === [c0 s1, s0 s1, c1]
    // n = [c0 c1, - s0 c1, s1]
    ym::make_triangles(usteps, vsteps, mtriangles, mpos, mnorm, mtexcoord,
        [radius](const ym::vec2f& uv) {
            auto a = ym::vec2f{2 * ym::pif * uv[0], ym::pif * (1 - radius)};
            auto r = (1 - uv[1]) + uv[1] * radius;
            return ym::vec3f{r * std::cos(a[0]) * std::sin(a[1]),
                r * std::sin(a[0]) * std::sin(a[1]), r * std::cos(a[1])};
        },
        [radius](const ym::vec2f& uv) {
            auto a = ym::vec2f{2 * ym::pif * uv[0], ym::pif * (1 - radius)};
            return ym::vec3f{-std::cos(a[0]) * std::cos(a[1]),
                -std::sin(a[0]) * std::cos(a[1]), std::sin(a[1])};
        },
        [radius](const ym::vec2f& uv) {
            return ym::vec2f{uv[0], radius + (1 - radius) * uv[1]};
        });
    ym::merge_triangles(
        triangles, pos, norm, texcoord, mtriangles, mpos, mnorm, mtexcoord);
}

//
// Make standard shape. Public API described above.
//
void make_uvhollowcutsphere1(int usteps, int vsteps, float radius,
    std::vector<ym::vec3i>& triangles, std::vector<ym::vec3f>& pos,
    std::vector<ym::vec3f>& norm, std::vector<ym::vec2f>& texcoord) {
    triangles.clear();
    pos.clear();
    norm.clear();
    texcoord.clear();

    std::vector<ym::vec3f> mpos, mnorm;
    std::vector<ym::vec2f> mtexcoord;
    std::vector<ym::vec3i> mtriangles;
    make_uvcutsphere(
        usteps, vsteps, radius, mtriangles, mpos, mnorm, mtexcoord);
    for (auto& uv : mtexcoord) uv.y *= radius;
    for (auto i = (usteps + 1) * vsteps; i < mnorm.size(); i++)
        mnorm[i] = normalize(mnorm[i] + ym::vec3f{0, 0, 1});
    ym::merge_triangles(
        triangles, pos, norm, texcoord, mtriangles, mpos, mnorm, mtexcoord);
    make_uvflippedcutsphere(
        usteps, vsteps, radius * 1.05f, mtriangles, mpos, mnorm, mtexcoord);
    for (auto& p : mpos) p *= 0.8f;
    ym::merge_triangles(
        triangles, pos, norm, texcoord, mtriangles, mpos, mnorm, mtexcoord);
    ym::make_triangles(usteps, vsteps / 4, mtriangles, mpos, mnorm, mtexcoord,
        [radius](const ym::vec2f& uv) {
            auto p = 1 - std::acos(radius) / ym::pif;
            auto v = p + uv[1] * (1 - p);
            auto a = ym::vec2f{2 * ym::pif * uv[0], ym::pif * (1 - v)};
            return ym::vec3f{std::cos(a[0]) * std::sin(a[1]),
                std::sin(a[0]) * std::sin(a[1]), (2 * radius - std::cos(a[1]))};
        },
        [radius](const ym::vec2f& uv) {
            auto p = 1 - std::acos(radius) / ym::pif;
            auto v = p + uv[1] * (1 - p);
            auto a = ym::vec2f{2 * ym::pif * uv[0], ym::pif * (1 - v)};
            return ym::vec3f{-std::cos(a[0]) * std::sin(a[1]),
                -std::sin(a[0]) * std::sin(a[1]), std::cos(a[1])};
        },
        [radius](const ym::vec2f& uv) {
            return ym::vec2f{uv[0], radius + (1 - radius) * uv[1]};
        });
    for (auto i = 0; i < (usteps + 1); i++)
        mnorm[i] = normalize(mnorm[i] + ym::vec3f{0, 0, 1});
    ym::merge_triangles(
        triangles, pos, norm, texcoord, mtriangles, mpos, mnorm, mtexcoord);
}

template <typename T>
std::vector<T>& operator+=(std::vector<T>& a, const std::vector<T>& b) {
    for (auto aa : b) a.push_back(aa);
    return a;
}

template <typename T>
std::vector<T>& operator+=(std::vector<T>& a, const T& b) {
    a.push_back(b);
    return a;
}

template <typename T>
std::vector<T> operator+(const std::vector<T>& a, const std::vector<T>& b) {
    auto c = a;
    c += b;
    return c;
}

template <typename T>
std::vector<T> operator+(const T& a, const std::vector<T>& b) {
    auto c = std::vector<T*>{a};
    c += b;
    return c;
}

template <typename T>
std::vector<T> operator+(const std::vector<T>& a, const T& b) {
    auto c = a;
    c += b;
    return c;
}

ym::frame3f make_lookat_frame(const ym::vec3f& pos, const ym::vec3f& to) {
    auto xf = lookat_frame3(pos, to, {0, 1, 0});
    xf[2] = -xf[2];
    xf[0] = -xf[0];
    return xf;
}

yobj::texture* add_texture(yobj::scene* scn, const std::string& path) {
    for (auto txt : scn->textures)
        if (txt->path == path) return txt;
    scn->textures += new yobj::texture();
    auto txt = scn->textures.back();
    txt->path = path;
    return txt;
}

yobj::material* add_material(yobj::scene* scn, const std::string& name,
    const ym::vec3f& ke, const ym::vec3f& kd, const ym::vec3f& ks,
    const ym::vec3f& kt, float rs, float op, yobj::texture* ke_txt,
    yobj::texture* kd_txt, yobj::texture* ks_txt, yobj::texture* kt_txt,
    yobj::texture* norm_txt) {
    for (auto mat : scn->materials)
        if (mat->name == name) return mat;
    scn->materials += new yobj::material();
    auto mat = scn->materials.back();
    mat->name = name;
    mat->ke = ke;
    mat->kd = kd;
    mat->ks = ks;
    mat->kt = kt;
    mat->rs = rs;
    mat->opacity = op;
    mat->ke_txt = ke_txt;
    mat->kd_txt = kd_txt;
    mat->ks_txt = ks_txt;
    mat->kt_txt = kt_txt;
    mat->norm_txt = norm_txt;
    return mat;
}

yobj::material* add_emission(yobj::scene* scn, const std::string& name,
    const ym::vec3f& ke, yobj::texture* txt = nullptr,
    yobj::texture* norm = nullptr) {
    auto mat = add_material(scn, name, ke, ym::zero3f, ym::zero3f, ym::zero3f,
        0, 1, txt, nullptr, nullptr, nullptr, norm);
    mat->unknown_props["PBR_HACK"] = {"emission"};
    return mat;
}

yobj::material* add_diffuse(yobj::scene* scn, const std::string& name,
    const ym::vec3f& kd, yobj::texture* txt = nullptr,
    yobj::texture* norm = nullptr) {
    auto mat = add_material(scn, name, ym::zero3f, kd, ym::zero3f, ym::zero3f,
        0, 1, nullptr, txt, nullptr, nullptr, norm);
    mat->unknown_props["PBR_HACK"] = {"diffuse"};
    return mat;
}

yobj::material* add_plastic(yobj::scene* scn, const std::string& name,
    const ym::vec3f& kd, float rs, yobj::texture* txt = nullptr,
    yobj::texture* norm = nullptr) {
    auto mat = add_material(scn, name, ym::zero3f, kd, {0.04f, 0.04f, 0.04f},
        ym::zero3f, rs, 1, nullptr, txt, nullptr, nullptr, norm);
    mat->unknown_props["PBR_HACK"] = {"plastic"};
    return mat;
}

yobj::material* add_metal(yobj::scene* scn, const std::string& name,
    const ym::vec3f& kd, float rs, yobj::texture* txt = nullptr,
    yobj::texture* norm = nullptr) {
    auto mat = add_material(scn, name, ym::zero3f, ym::zero3f, kd, ym::zero3f,
        rs, 1, nullptr, nullptr, txt, nullptr, norm);
    mat->unknown_props["PBR_HACK"] = {"metal"};
    return mat;
}

yobj::material* add_glass(yobj::scene* scn, const std::string& name,
    const ym::vec3f& kd, float rs, yobj::texture* txt = nullptr,
    yobj::texture* norm = nullptr) {
    auto mat = add_material(scn, name, ym::zero3f, ym::zero3f,
        {0.04f, 0.04f, 0.04f}, kd, rs, 1, nullptr, nullptr, txt, nullptr, norm);
    mat->unknown_props["PBR_HACK"] = {"glass"};
    return mat;
}

yobj::material* add_transparent_diffuse(yobj::scene* scn,
    const std::string& name, const ym::vec3f& kd, float op,
    yobj::texture* txt = nullptr, yobj::texture* norm = nullptr) {
    auto mat = add_material(scn, name, ym::zero3f, kd, ym::zero3f, ym::zero3f,
        0, op, nullptr, txt, nullptr, nullptr, norm);
    mat->unknown_props["PBR_HACK"] = {"diffuse"};
    return mat;
}

yobj::material* add_transparent_plastic(yobj::scene* scn,
    const std::string& name, const ym::vec3f& kd, float rs, float op,
    yobj::texture* txt = nullptr, yobj::texture* norm = nullptr) {
    auto mat = add_material(scn, name, ym::zero3f, kd, {0.04f, 0.04f, 0.04f},
        ym::zero3f, rs, op, nullptr, txt, nullptr, nullptr, norm);
    mat->unknown_props["PBR_HACK"] = {"plastic"};
    return mat;
}

ym::vec3f srgb(const ym::vec3f& k) {
    return {std::pow(k[0] / 255.0f, 2.2f), std::pow(k[1] / 255.0f, 2.2f),
        std::pow(k[2] / 255.0f, 2.2f)};
}

yobj::material* add_material(yobj::scene* scn, const std::string& name) {
    if (name == "def") {
        return add_diffuse(scn, name, {0.2f, 0.2f, 0.2f});
    } else if (name == "matte00") {
        return add_diffuse(scn, name, {0.2f, 0.2f, 0.2f});
    } else if (name == "matte01") {
        return add_diffuse(scn, name, {0.5f, 0.2f, 0.2f});
    } else if (name == "matte02") {
        return add_diffuse(scn, name, {0.2f, 0.5f, 0.2f});
    } else if (name == "matte03") {
        return add_diffuse(scn, name, {0.2f, 0.2f, 0.5f});
    } else if (name == "matte04") {
        return add_diffuse(scn, name, {0.5f, 0.5f, 0.5f});
    } else if (name == "floor") {
        return add_diffuse(scn, name, {0.2f, 0.2f, 0.2f});
    } else if (name == "matte00_txt") {
        return add_diffuse(scn, name, {1, 1, 1}, add_texture(scn, "grid.png"));
    } else if (name == "matte01_txt") {
        return add_diffuse(
            scn, name, {1, 1, 1}, add_texture(scn, "rcolored.png"));
    } else if (name == "matte02_txt") {
        return add_diffuse(
            scn, name, {1, 1, 1}, add_texture(scn, "checker.png"));
    } else if (name == "matte03_txt") {
        return add_diffuse(
            scn, name, {1, 1, 1}, add_texture(scn, "colored.png"));
    } else if (name == "floor_txt") {
        return add_diffuse(scn, name, {1, 1, 1}, add_texture(scn, "grid.png"));
    } else if (name == "plastic00") {
        return add_plastic(scn, name, {0.2f, 0.2f, 0.2f}, 0.01f);
    } else if (name == "plastic01") {
        return add_plastic(scn, name, {0.5f, 0.2f, 0.2f}, 0.1f);
    } else if (name == "plastic02") {
        return add_plastic(scn, name, {0.2f, 0.5f, 0.2f}, 0.05f);
    } else if (name == "plastic03") {
        return add_plastic(scn, name, {0.2f, 0.2f, 0.5f}, 0.01f);
    } else if (name == "plastic04") {
        return add_plastic(scn, name, {0.5f, 0.5f, 0.5f}, 0.01f);
    } else if (name == "plastic00_txt") {
        return add_plastic(
            scn, name, {1, 1, 1}, 0.1f, add_texture(scn, "grid.png"));
    } else if (name == "plastic01_txt") {
        return add_plastic(
            scn, name, {1, 1, 1}, 0.1f, add_texture(scn, "rcolored.png"));
    } else if (name == "plastic02_txt") {
        return add_plastic(
            scn, name, {1, 1, 1}, 0.05f, add_texture(scn, "checker.png"));
    } else if (name == "plastic03_txt") {
        return add_plastic(
            scn, name, {1, 1, 1}, 0.01f, add_texture(scn, "colored.png"));
    } else if (name == "plastic04_txt") {
        return add_plastic(scn, name, {0.2f, 0.2f, 0.2f}, 0.01f, nullptr,
            add_texture(scn, "grid_normal.png"));
    } else if (name == "plastic05_txt") {
        return add_plastic(scn, name, {0.2f, 0.2f, 0.2f}, 0.01f, nullptr,
            add_texture(scn, "../data/menzel/stone-12_norm_cropped.png"));
    } else if (name == "plastic06_txt") {
        return add_plastic(
            scn, name, {1, 1, 1}, 0.01f, add_texture(scn, "colored.png"));
    } else if (name == "metal00") {
        return add_metal(scn, name, {0.8f, 0.8f, 0.8f}, 0);
    } else if (name == "metal01") {
        return add_metal(scn, name, {0.8f, 0.8f, 0.8f}, 0);
    } else if (name == "metal02") {
        return add_metal(scn, name, {0.8f, 0.8f, 0.8f}, 0.01f);
    } else if (name == "metal03") {
        return add_metal(scn, name, {0.8f, 0.8f, 0.8f}, 0.05f);
    } else if (name == "gold01") {
        return add_metal(scn, name, srgb({245, 215, 121}), 0.01f);
    } else if (name == "gold02") {
        return add_metal(scn, name, srgb({245, 215, 121}), 0.05f);
    } else if (name == "silver01") {
        return add_metal(scn, name, srgb({252, 250, 246}), 0.01f);
    } else if (name == "silver02") {
        return add_metal(scn, name, srgb({252, 250, 246}), 0.05f);
    } else if (name == "iron01") {
        return add_metal(scn, name, srgb({195, 199, 199}), 0.01f);
    } else if (name == "iron02") {
        return add_metal(scn, name, srgb({195, 199, 199}), 0.05f);
    } else if (name == "copper01") {
        return add_metal(scn, name, srgb({250, 190, 160}), 0.01f);
    } else if (name == "copper02") {
        return add_metal(scn, name, srgb({250, 190, 160}), 0.05f);
    } else if (name == "bump00") {
        return add_plastic(scn, name, {0.5f, 0.5f, 0.5f}, 0.05f, nullptr,
            add_texture(scn, "grid_normal.png"));
    } else if (name == "bump01") {
        return add_metal(scn, name, srgb({250, 190, 160}), 0.01f, nullptr,
            add_texture(scn, "grid_normal.png"));
    } else if (name == "bump02") {
        return add_plastic(scn, name, {0.2f, 0.2f, 0.5f}, 0.05f, nullptr,
            add_texture(scn, "grid_normal.png"));
    } else if (name == "bump03") {
        return add_metal(scn, name, srgb({250, 190, 160}), 0.05f, nullptr,
            add_texture(scn, "grid_normal.png"));
    } else if (name == "glass00") {
        return add_glass(scn, name, {0.8f, 0.8f, 0.8f}, 0);
    } else if (name == "glass01") {
        return add_glass(scn, name, {0.8f, 0.8f, 0.8f}, 0);
    } else if (name == "glass02") {
        return add_glass(scn, name, {0.8f, 0.8f, 0.8f}, 0.01f);
    } else if (name == "glass03") {
        return add_glass(scn, name, {0.8f, 0.8f, 0.8f}, 0.05f);
    } else if (name == "lines00") {
        return add_diffuse(scn, name, {0.2f, 0.2f, 0.2f});
    } else if (name == "lines01") {
        return add_diffuse(scn, name, {0.2f, 0.2f, 0.2f});
    } else if (name == "lines02") {
        return add_diffuse(scn, name, {0.2f, 0.2f, 0.2f});
    } else if (name == "lines03") {
        return add_diffuse(scn, name, {0.2f, 0.2f, 0.2f});
    } else if (name == "lines01_txt") {
        return add_diffuse(scn, name, {0.2f, 0.2f, 0.2f});
    } else if (name == "lines02_txt") {
        return add_diffuse(scn, name, {0.2f, 0.2f, 0.2f});
    } else if (name == "lines03_txt") {
        return add_diffuse(scn, name, {0.2f, 0.2f, 0.2f});
    } else if (name == "points00") {
        return add_diffuse(scn, name, {0.2f, 0.2f, 0.2f});
    } else if (name == "points01") {
        return add_diffuse(scn, name, {0.2f, 0.2f, 0.2f});
    } else if (name == "points01_txt") {
        return add_diffuse(scn, name, {0.2f, 0.2f, 0.2f});
    } else if (name == "pointlight") {
        return add_emission(scn, name, {400, 400, 400});
    } else if (name == "arealight") {
        return add_emission(scn, name, {40, 40, 40});
    } else if (name == "arealight_low") {
        return add_emission(scn, name, {16, 16, 16});
    } else if (name == "env") {
        return add_emission(scn, name, {1, 1, 1});
    } else if (name == "env_txt") {
        return add_emission(scn, name, {1, 1, 1}, add_texture(scn, "env.hdr"));
    } else if (name == "sym_points") {
        return add_diffuse(scn, name, {0.2f, 0.8f, 0.2f});
    } else if (name == "sym_lines") {
        return add_diffuse(scn, name, {0.2f, 0.8f, 0.2f});
    } else if (name == "sym_cloth") {
        return add_diffuse(scn, name, {0.2f, 0.8f, 0.2f});
    } else {
        throw std::runtime_error("bad value");
    }
    return nullptr;
}

yobj::mesh* add_mesh(yobj::scene* scn, yobj::shape* shp) {
    auto msh = new yobj::mesh();
    msh->name = shp->name;
    msh->shapes += shp;
    scn->meshes += msh;
    return msh;
}

yobj::instance* add_instance(yobj::scene* scn, const std::string& name,
    yobj::mesh* shp, const ym::vec3f& pos = {0, 0, 0},
    const ym::vec3f& rot = {0, 0, 0}) {
    scn->instances += new yobj::instance();
    scn->instances.back()->name = name;
    scn->instances.back()->msh = shp;
    scn->instances.back()->translation = pos;
    if (rot != ym::zero3f) {
        ym::mat3f xf = ym::identity_mat3f;
        xf = rotation_mat3(ym::vec3f{1, 0, 0}, rot[0] * ym::pif / 180) * xf;
        xf = rotation_mat3(ym::vec3f{0, 1, 0}, rot[1] * ym::pif / 180) * xf;
        xf = rotation_mat3(ym::vec3f{0, 0, 1}, rot[2] * ym::pif / 180) * xf;
        scn->instances.back()->rotation = ym::rotation_quat4(xf);
    }
    return scn->instances.back();
}

yobj::instance* add_instance(yobj::scene* scn, const std::string& name,
    yobj::mesh* shp, const ym::vec3f& pos, const ym::quat4f& rot) {
    scn->instances += new yobj::instance();
    scn->instances.back()->name = name;
    scn->instances.back()->msh = shp;
    scn->instances.back()->matrix =
        ym::translation_mat4(pos) * ym::rotation_mat4(rot);
    return scn->instances.back();
}

yobj::instance* add_instance(yobj::scene* scn, const std::string& name,
    yobj::mesh* shp, const ym::vec3f& pos, const ym::mat3f& rot) {
    scn->instances += new yobj::instance();
    scn->instances.back()->name = name;
    scn->instances.back()->msh = shp;
    scn->instances.back()->translation = pos;
    scn->instances.back()->rotation = ym::rotation_quat4(rot);
    return scn->instances.back();
}

yobj::instance* add_instance(yobj::scene* scn, const std::string& name,
    yobj::shape* shp, const ym::vec3f& pos = {0, 0, 0},
    const ym::vec3f& rot = {0, 0, 0}) {
    return add_instance(scn, name, add_mesh(scn, shp), pos, rot);
}

yobj::mesh* add_box(yobj::scene* scn, const std::string& name,
    yobj::material* mat, int level, const ym::vec3f& scale) {
    auto shp = new yobj::shape();
    shp->mat = mat;
    shp->name = name;
    auto usteps = ym::pow2(level), vsteps = ym::pow2(level);
    ym::make_uvcube(
        usteps, vsteps, shp->triangles, shp->pos, shp->norm, shp->texcoord);
    for (auto& p : shp->pos) p *= scale;
    return add_mesh(scn, shp);
}

yobj::mesh* add_cube(yobj::scene* scn, const std::string& name,
    yobj::material* mat, int level, float scale = 1) {
    auto shp = new yobj::shape();
    shp->mat = mat;
    shp->name = name;
    auto usteps = ym::pow2(level), vsteps = ym::pow2(level);
    ym::make_uvcube(
        usteps, vsteps, shp->triangles, shp->pos, shp->norm, shp->texcoord);
    for (auto& p : shp->pos) p *= scale;
    return add_mesh(scn, shp);
}

yobj::mesh* add_quad(yobj::scene* scn, const std::string& name,
    yobj::material* mat, int level, float scale = 1) {
    auto shp = new yobj::shape();
    shp->mat = mat;
    shp->name = name;
    auto usteps = ym::pow2(level), vsteps = ym::pow2(level);
    ym::make_uvquad(
        usteps, vsteps, shp->triangles, shp->pos, shp->norm, shp->texcoord);
    for (auto& p : shp->pos) p *= scale;
    return add_mesh(scn, shp);
}

yobj::mesh* add_sphere(yobj::scene* scn, const std::string& name,
    yobj::material* mat, int level, float scale = 1) {
    auto shp = new yobj::shape();
    shp->mat = mat;
    shp->name = name;
    auto usteps = ym::pow2(level + 2), vsteps = ym::pow2(level + 1);
    ym::make_uvsphere(
        usteps, vsteps, shp->triangles, shp->pos, shp->norm, shp->texcoord);
    for (auto& p : shp->pos) p *= scale;
    return add_mesh(scn, shp);
}

yobj::mesh* add_flipcapsphere(yobj::scene* scn, const std::string& name,
    yobj::material* mat, int level, float scale = 1) {
    auto shp = new yobj::shape();
    shp->mat = mat;
    shp->name = name;
    auto usteps = ym::pow2(level + 2), vsteps = ym::pow2(level + 1);
    ym::make_uvflipcapsphere(usteps, vsteps, 0.75f, shp->triangles, shp->pos,
        shp->norm, shp->texcoord);
    for (auto& p : shp->pos) p *= scale;
    return add_mesh(scn, shp);
}

yobj::mesh* add_spherecube(yobj::scene* scn, const std::string& name,
    yobj::material* mat, int level, float scale = 1) {
    auto shp = new yobj::shape();
    shp->mat = mat;
    shp->name = name;
    auto usteps = ym::pow2(level), vsteps = ym::pow2(level);
    ym::make_uvspherecube(
        usteps, vsteps, shp->triangles, shp->pos, shp->norm, shp->texcoord);
    for (auto& p : shp->pos) p *= scale;
    return add_mesh(scn, shp);
}

yobj::mesh* add_spherizedcube(yobj::scene* scn, const std::string& name,
    yobj::material* mat, int level, float scale = 1) {
    auto shp = new yobj::shape();
    shp->mat = mat;
    shp->name = name;
    auto usteps = ym::pow2(level), vsteps = ym::pow2(level);
    ym::make_uvspherizedcube(usteps, vsteps, 0.75f, shp->triangles, shp->pos,
        shp->norm, shp->texcoord);
    for (auto& p : shp->pos) p *= scale;
    return add_mesh(scn, shp);
}

yobj::mesh* add_uvhollowcutsphere(yobj::scene* scn, const std::string& name,
    yobj::material* mat, int level, float scale = 1) {
    auto shp = new yobj::shape();
    shp->mat = mat;
    shp->name = name;
    auto usteps = ym::pow2(level), vsteps = ym::pow2(level);
    make_uvhollowcutsphere(usteps, vsteps, 0.75f, shp->triangles, shp->pos,
        shp->norm, shp->texcoord);
    for (auto& p : shp->pos) p *= scale;
    return add_mesh(scn, shp);
}

yobj::mesh* add_uvhollowcutsphere1(yobj::scene* scn, const std::string& name,
    yobj::material* mat, int level, float scale = 1) {
    auto shp = new yobj::shape();
    shp->mat = mat;
    shp->name = name;
    auto usteps = ym::pow2(level), vsteps = ym::pow2(level);
    make_uvhollowcutsphere1(usteps, vsteps, 0.75f, shp->triangles, shp->pos,
        shp->norm, shp->texcoord);
    for (auto& p : shp->pos) p *= scale;
    return add_mesh(scn, shp);
}

yobj::mesh* add_floor(yobj::scene* scn, const std::string& name,
    yobj::material* mat, float s = 6, float p = 4, int l = 6,
    float scale = 12) {
    auto n = (int)round(powf(2, (float)l));
    auto shp = new yobj::shape();
    shp->name = name;
    shp->mat = mat;
    ym::make_triangles(n, n, shp->triangles, shp->pos, shp->norm, shp->texcoord,
        [p, scale](const ym::vec2f& uv) {
            auto pos = ym::zero3f;
            auto x = 2 * uv[0] - 1;
            auto y = 2 * (1 - uv[1]) - 1;
            if (y >= 0 || !p) {
                pos = {x, 0, y};
            } else {
                pos = {x, std::pow(-y, p), y};
            }
            return scale * pos;
        },
        [](const ym::vec2f& uv) {
            return ym::vec3f{0, 1, 0};
        },
        [s](const ym::vec2f& uv) { return uv * s; });
    if (p) { ym::compute_normals(shp->triangles, shp->pos, shp->norm); }
    return add_mesh(scn, shp);
}

yobj::mesh* add_lines(yobj::scene* scn, const std::string& name,
    yobj::material* mat, int num, int n, float r, float c, float s) {
    auto shp = new yobj::shape();
    shp->mat = mat;
    shp->name = name;

    ym::rng_pcg32 rn;
    std::vector<ym::vec3f> base(num + 1), dir(num + 1);
    std::vector<float> ln(num + 1);
    for (auto i = 0; i <= num; i++) {
        auto z = -1 + 2 * next1f(&rn);
        auto r = std::sqrt(ym::clamp(1 - z * z, (float)0, (float)1));
        auto phi = 2 * ym::pif * next1f(&rn);
        base[i] = ym::vec3f{r * std::cos(phi), r * std::sin(phi), z};
        dir[i] = base[i];
        ln[i] = 0.15f + 0.15f * next1f(&rn);
    }

    ym::make_lines(num, n, shp->lines, shp->pos, shp->norm, shp->texcoord,
        shp->radius,
        [num, base, dir, ln, r, s, c, &rn](int idx, float u) {
            auto pos = base[idx] * (1 + u * ln[idx]);
            if (r) {
                pos += ym::vec3f{r * (0.5f - next1f(&rn)),
                    r * (0.5f - next1f(&rn)), r * (0.5f - next1f(&rn))};
            }
            if (s && u) {
                ym::frame3f rotation =
                    rotation_frame3(ym::vec3f{0, 1, 0}, s * u * u);
                pos = transform_point(rotation, pos);
            }
            auto nc = 128;
            if (c && idx > nc) {
                int cc = 0;
                float md = HUGE_VALF;
                for (int k = 0; k < nc; k++) {
                    float d = dist(base[idx], base[k]);
                    if (d < md) {
                        md = d;
                        cc = k;
                    }
                }
                ym::vec3f cpos = base[cc] * (1 + u * ln[cc]);
                pos = pos * (1 - c * u * u) + cpos * (c * u * u);
            }
            return pos;
        },
        [](int idx, float u) {
            return ym::vec3f{0, 0, 1};
        },
        [num](int idx, float u) {
            return ym::vec2f{u, (float)idx / (float)num};
        },
        [](int idx, float u) { return 0.001f + 0.001f * (1 - u); });

    ym::compute_tangents(shp->lines, shp->pos, shp->norm);
    return add_mesh(scn, shp);
}

yobj::mesh* add_points(yobj::scene* scn, const std::string& name,
    yobj::material* mat, int num, int seed) {
    auto shp = new yobj::shape();
    shp->mat = mat;
    shp->name = name;
    ym::rng_pcg32 rn;
    ym::init(&rn, seed, 0);
    ym::make_points(num, shp->points, shp->pos, shp->norm, shp->texcoord,
        shp->radius,
        [&rn](float u) {
            return ym::vec3f{-1 + 2 * next1f(&rn), -1 + 2 * next1f(&rn),
                -1 + 2 * next1f(&rn)};
        },
        [](float u) {
            return ym::vec3f{0, 0, 1};
        },
        [](float u) {
            return ym::vec2f{u, 0};
        },
        [](float u) { return 0.0025f; });
    return add_mesh(scn, shp);
}

yobj::instance* add_points_instance(yobj::scene* scn, const std::string& name,
    yobj::material* mat, int num, int seed, const ym::vec3f& pos = {0, 0, 0},
    const ym::vec3f& rot = {0, 0, 0}) {
    return add_instance(
        scn, name, add_points(scn, name, mat, num, seed), pos, rot);
}

yobj::mesh* add_point(yobj::scene* scn, const std::string& name,
    yobj::material* mat, float radius = 0.001f) {
    auto shp = new yobj::shape();
    shp->mat = mat;
    shp->name = name;
    shp->points.push_back(0);
    shp->pos.push_back({0, 0, 0});
    shp->norm.push_back({0, 0, 1});
    shp->radius.push_back(radius);
    return add_mesh(scn, shp);
}

yobj::environment* add_env(yobj::scene* scn, const std::string& name,
    yobj::material* mat,
    const ym::frame3f& frame = make_lookat_frame({0, 0, 0}, {-1.5f, 0, 0})) {
    scn->environments += new yobj::environment();
    auto env = scn->environments.back();
    env->name = name;
    env->mat = mat;
    // TODO
    env->matrix = ym::to_mat(frame);
    return env;
}

yobj::camera* add_camera(yobj::scene* scn, const std::string& name,
    const ym::vec3f& from, const ym::vec3f& to, float h, float a,
    float r = 16.0f / 9.0f) {
    scn->cameras += new yobj::camera();
    auto cam = scn->cameras.back();
    cam->name = name;
    // TODO
    cam->matrix = to_mat(lookat_frame3(from, to, {0, 1, 0}));
    cam->aperture = a;
    cam->focus = dist(from, to);
    cam->yfov = 2 * atan(h / 2);
    cam->aspect = r;
    return cam;
}

using byte = unsigned char;

#define sqr(x) ((x) * (x))

ym::image<ym::vec4f> make_sunsky_hdr(int w, int h, float sun_theta,
    float turbidity, ym::vec3f ground, float scale, bool include_ground) {
    ym::image<ym::vec4f> rgba(w, h);
    ArHosekSkyModelState* skymodel_state[3] = {
        arhosek_rgb_skymodelstate_alloc_init(turbidity, ground[0], sun_theta),
        arhosek_rgb_skymodelstate_alloc_init(turbidity, ground[0], sun_theta),
        arhosek_rgb_skymodelstate_alloc_init(turbidity, ground[0], sun_theta),
    };
    auto sun_phi = ym::pif;
    auto sun_w = ym::vec3f{cosf(sun_phi) * sinf(sun_theta),
        sinf(sun_phi) * sinf(sun_theta), cosf(sun_theta)};
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            auto theta = ym::pif * (j + 0.5f) / h;
            auto phi = 2 * ym::pif * (i + 0.5f) / w;
            if (include_ground)
                theta = ym::clamp(theta, 0.0f, ym::pif / 2 - 0.001f);
            auto pw = ym::vec3f{std::cos(phi) * std::sin(theta),
                std::sin(phi) * std::sin(theta), std::cos(theta)};
            auto gamma =
                std::acos(ym::clamp(dot(sun_w, pw), (float)-1, (float)1));
            auto sky = ym::vec3f{(float)(arhosek_tristim_skymodel_radiance(
                                     skymodel_state[0], theta, gamma, 0)),
                (float)(arhosek_tristim_skymodel_radiance(
                    skymodel_state[1], theta, gamma, 1)),
                (float)(arhosek_tristim_skymodel_radiance(
                    skymodel_state[2], theta, gamma, 2))};
            rgba.at(i, j) = {scale * sky[0], scale * sky[1], scale * sky[2], 1};
        }
    }
    arhosekskymodelstate_free(skymodel_state[0]);
    arhosekskymodelstate_free(skymodel_state[1]);
    arhosekskymodelstate_free(skymodel_state[2]);
    return rgba;
}

void save_image(const std::string& filename, const std::string& dirname,
    const ym::image4b& img) {
    std::string path = std::string(dirname) + "/" + std::string(filename);
    yimg::save_image4b(path, img);
}

void save_image(const std::string& filename, const std::string& dirname,
    const ym::image4f& img) {
    std::string path = std::string(dirname) + "/" + std::string(filename);
    yimg::save_image4f(path, img);
}

std::vector<yobj::camera*> add_simple_cameras(yobj::scene* scn) {
    return {add_camera(scn, "cam", {0, 3, 10}, {0, 0, 0}, 0.5f, 0),
        add_camera(scn, "cam_dof", {0, 3, 10}, {0, 0, 0}, 0.5f, 0.1f)};
}

std::vector<yobj::camera*> add_matball_cameras(yobj::scene* scn) {
    return {add_camera(scn, "cam_close", {0, 5, 10}, {0, 0, 0}, 0.25f, 0, 1),
        add_camera(scn, "cam_close", {0, 3, 10}, {0, 0, 0}, 0.25f, 0, 1)};
}

// http://graphics.cs.williams.edu/data
// http://www.graphics.cornell.edu/online/box/data.html
yobj::scene* make_cornell_box_scene() {
    auto scn = new yobj::scene();
    std::vector<yobj::camera*> cameras = {
        add_camera(scn, "cb_cam", {0, 1, 4}, {0, 1, 0}, 0.7f, 0, 1)};
    std::vector<yobj::material*> materials = {
        add_diffuse(scn, "cb_white", {0.725f, 0.71f, 0.68f}),
        add_diffuse(scn, "cb_red", {0.63f, 0.065f, 0.05f}),
        add_diffuse(scn, "cb_green", {0.14f, 0.45f, 0.091f}),
        add_emission(scn, "cb_light", {17, 12, 4}),
    };
    std::vector<yobj::instance*> instances = {
        add_instance(scn, "cb_floor",
            add_quad(scn, "cb_floor", materials[0], 0), ym::zero3f,
            {-90, 0, 0}),
        add_instance(scn, "cb_ceiling",
            add_quad(scn, "cb_ceiling", materials[0], 0), {0, 2, 0},
            {90, 0, 0}),
        add_instance(scn, "cb_back", add_quad(scn, "cb_back", materials[0], 0),
            {0, 1, -1}),
        add_instance(scn, "cb_back", add_quad(scn, "cb_back", materials[2], 0),
            {+1, 1, 0}, {0, -90, 0}),
        add_instance(scn, "cb_back", add_quad(scn, "cb_back", materials[1], 0),
            {-1, 1, 0}, {0, 90, 0}),
        add_instance(scn, "cb_tallbox",
            add_box(scn, "cb_tallbox", materials[0], 0, {0.3f, 0.6f, 0.3f}),
            {-0.33f, 0.6f, -0.29f}, {0, 15, 0}),
        add_instance(scn, "cb_shortbox",
            add_box(scn, "cb_shortbox", materials[0], 0, {0.3f, 0.3f, 0.3f}),
            {0.33f, 0.3f, 0.33f}, {0, -15, 0}),
        add_instance(scn, "cb_light",
            add_quad(scn, "cb_light", materials[3], 0, 0.25f), {0, 1.999f, 0},
            {90, 0, 0})};
    return scn;
}

yobj::scene* make_matball_scene(const std::string& otype,
    const std::string& mtype, const std::string& ltype) {
    auto scn = new yobj::scene();
    add_camera(scn, "cam_close", {0, 5, 10}, {0, 0, 0}, 0.25f, 0, 1);
    add_camera(scn, "cam_close", {0, 3, 10}, {0, 0, 0}, 0.25f, 0, 1);

    add_instance(scn, "floor",
        add_floor(scn, "floor", add_material(scn, "floor_txt")), {0, -1, 0});

    auto mat = add_material(scn, mtype);

    if (otype == "matball01") {
        add_instance(scn, "intmatball",
            add_sphere(
                scn, "intmatball", add_material(scn, "matte00"), 5, 0.8f));
        add_instance(scn, "matball",
            add_uvhollowcutsphere(scn, "matball", mat, 5), {0, 0, 0},
            {0, 35, 45});

    } else if (otype == "matball02") {
        add_instance(scn, "intmatball",
            add_sphere(
                scn, "intmatball", add_material(scn, "matte00"), 5, 0.8f));
        add_instance(scn, "matball",
            add_uvhollowcutsphere1(scn, "matball", mat, 5), {0, 0, 0},
            {0, 35, 45});
    } else {
        throw std::runtime_error("bad value");
    }

    if (ltype == "pointlight") {
        auto mat = add_emission(scn, "pointlight", {400, 400, 400});
        add_instance(scn, "pointlight01", add_point(scn, "pointlight01", mat),
            {1.4f, 8, 6});
        add_instance(scn, "pointlight02", add_point(scn, "pointlight02", mat),
            {-1.4f, 8, 6});
    } else if (ltype == "arealight") {
        auto mat = add_emission(scn, "arealight", {40, 40, 40});
        auto shp = add_quad(scn, "arealight", mat, 0, 2);
        add_instance(scn, "arealight01", shp, {-4, 4, 8},
            make_lookat_frame({-4, 4, 8}, {0, 2, 0}).rot());
        add_instance(scn, "arealight02", shp, {4, 4, 8},
            make_lookat_frame({4, 4, 8}, {0, 2, 0}).rot());
    } else if (ltype == "envlight") {
        auto mat = add_emission(
            scn, "envlight", {1, 1, 1}, add_texture(scn, "env.hdr"));
        add_env(scn, "envlight", mat);
    } else {
        throw std::runtime_error("bad value");
    }

    return scn;
}

yobj::instance* add_hair(yobj::scene* scn, yobj::instance* ist, int nhairs,
    int level, float length, uint64_t seed, yobj::material* mat) {
    auto hair_mesh = new yobj::mesh();
    hair_mesh->name = ist->msh->name + "_hair";
    for (auto shp : ist->msh->shapes) {
        auto hair_shp = new yobj::shape();
        hair_shp->name = shp->name;
        hair_shp->mat = mat;
        std::vector<ym::vec3f> triangle_pos;
        std::vector<ym::vec3f> triangle_norm;
        std::vector<ym::vec2f> triangle_texcoord;
        ym::sample_triangles_points(shp->triangles, shp->pos, shp->norm,
            shp->texcoord, nhairs, triangle_pos, triangle_norm,
            triangle_texcoord, seed);
        ym::make_lines(nhairs, ym::pow2(level), hair_shp->lines, hair_shp->pos,
            hair_shp->norm, hair_shp->texcoord, hair_shp->radius,
            [&triangle_pos, &triangle_norm, length](int idx, float u) {
                return triangle_pos[idx] + triangle_norm[idx] * u * length;
            },
            [&triangle_norm](int idx, float u) { return triangle_norm[idx]; },
            [nhairs](int idx, float u) {
                return ym::vec2f{u, (float)idx / (float)nhairs};
            },
            [](int idx, float u) { return ym::lerp(0.01f, 0.0f, u); });
        hair_mesh->shapes.push_back(hair_shp);
    }
    scn->meshes.push_back(hair_mesh);
    return add_instance(
        scn, ist->name + "_hair", hair_mesh, ist->translation, ist->rotation);
};

std::vector<yobj::instance*> add_random_instances(
    yobj::scene* scn, float cell_size, int ninstances, int nmeshes) {
    auto rng = ym::rng_pcg32();
    ym::init(&rng, 0, 0);
    auto meshes = std::vector<yobj::mesh*>();
    for (auto i = 0; i < nmeshes; i++) {
        auto mtype = ym::next(&rng) % 2;
        auto mat = (yobj::material*)nullptr;
        auto mname = yu::string::formatf("mat%03d", i);
        switch (mtype) {
            case 0:
                mat = add_diffuse(scn, mname, 0.5f + 0.5f * ym::next3f(&rng));
                break;
            case 1:
                mat = add_plastic(scn, mname, 0.5f + 0.5f * ym::next3f(&rng),
                    0.01f + 0.04f * ym::next1f(&rng));
                break;
        }
        auto stype = ym::next(&rng) % 3;
        auto sname = yu::string::formatf("shp%03d", i);
        auto msh = (yobj::mesh*)nullptr;
        switch (stype) {
            case 0: msh = add_sphere(scn, sname, mat, 4, 1); break;
            case 1: msh = add_cube(scn, sname, mat, 4, 1); break;
            case 2: msh = add_spherecube(scn, sname, mat, 4, 1); break;
        }
        meshes.push_back(msh);
    }
    auto num = (int)(ym::sqrt(ninstances) + 0.5f);
    auto instances = std::vector<yobj::instance*>();
    for (auto j = 0; j < num; j++) {
        for (auto i = 0; i < num; i++) {
            auto ist = (yobj::instance*)nullptr;
            auto iname = yu::string::formatf("ist%06d", i);
            auto mid = ym::next(&rng) % nmeshes;
            auto rxy =
                ym::vec3f{ym::next1f(&rng) - 0.5f, 0, ym::next1f(&rng) - 0.5f};
            auto cxy = ym::vec3f(i - num / 2, 0, j - num / 2);
            auto pos = cell_size * cxy + rxy;
            add_instance(scn, iname, meshes[mid], pos);
            instances.push_back(ist);
        }
    }
    return instances;
}

yobj::scene* make_instance_scene(
    const std::string& otype, const std::string& ltype) {
    auto scn = new yobj::scene();
    add_camera(scn, "cam01", {0, 75, 75}, {0, 0, 0}, 0.5f, 0);
    add_camera(scn, "cam02", {0, 150, 150}, {0, 0, 0}, 0.5f, 0);
    add_camera(scn, "cam03", {0, 15, 75}, {0, 0, 0}, 0.5f, 0);
    add_camera(scn, "cam04", {0, 30, 150}, {0, 0, 0}, 0.5f, 0);
    add_camera(scn, "cam_dof", {0, 75, 75}, {0, 0, 0}, 0.5f, 0.1f);

    add_instance(scn, "floor",
        add_floor(scn, "floor", add_material(scn, "floor_txt"), 12, 0, 0, 120),
        {0, -1, 0});

    if (otype == "instance100") {
        add_random_instances(scn, 4, 100, 10);
    } else if (otype == "instance1600") {
        add_random_instances(scn, 3, 1600, 10);
    } else if (otype == "instance2500") {
        add_random_instances(scn, 2, 2500, 10);
    } else if (otype == "instance10000") {
        add_random_instances(scn, 2, 10000, 10);
    } else {
        throw std::runtime_error("bad value");
    }

    if (ltype == "pointlight") {
        add_instance(scn, "pointlight01",
            add_point(scn, "pointlight01",
                add_emission(scn, "pointlight01", ym::vec3f{10000})),
            {0, 50, 50});
        add_instance(scn, "pointlight02",
            add_point(scn, "pointlight02",
                add_emission(scn, "pointlight02", ym::vec3f{4000})),
            {50, 50, 0});
        add_instance(scn, "pointlight03",
            add_point(scn, "pointlight03",
                add_emission(scn, "pointlight03", ym::vec3f{4000})),
            {0, 50, -50});
    } else if (ltype == "arealight") {
        auto mat = add_emission(scn, "arealight", {40, 40, 40});
        auto shp = add_quad(scn, "arealight", mat, 0, 2);
        add_instance(scn, "arealight01", shp, {-4, 4, 8},
            make_lookat_frame({-4, 4, 8}, {0, 2, 0}).rot());
        add_instance(scn, "arealight02", shp, {4, 4, 8},
            make_lookat_frame({4, 4, 8}, {0, 2, 0}).rot());
    } else if (ltype == "envlight") {
        auto mat = add_emission(
            scn, "envlight", {1, 1, 1}, add_texture(scn, "env.hdr"));
        add_env(scn, "envlight", mat);
    } else {
        throw std::runtime_error("bad value");
    }

    return scn;
}

yobj::scene* make_simple_scene(
    const std::string& otype, const std::string& ltype) {
    auto scn = new yobj::scene();
    add_camera(scn, "cam", {0, 3, 10}, {0, 0, 0}, 0.5f, 0);
    add_camera(scn, "cam_dof", {0, 3, 10}, {0, 0, 0}, 0.5f, 0.1f);

    add_instance(scn, "floor",
        add_floor(scn, "floor", add_material(scn, "floor_txt")), {0, -1, 0});

    auto add_objects = [](yobj::scene* scn, std::vector<yobj::material*> mat) {
        return std::vector<yobj::instance*>{
            add_instance(scn, "obj01",
                add_flipcapsphere(scn, "obj01", mat[0], 5), {-2.5f, 0, 0}),
            add_instance(
                scn, "obj02", add_spherizedcube(scn, "obj02", mat[1], 4)),
            add_instance(scn, "obj03", add_spherecube(scn, "obj03", mat[2], 4),
                {2.5f, 0, 0})};
    };

    if (otype == "basic") {
        auto mat = std::vector<yobj::material*>{
            add_plastic(scn, "obj01", {0.5f, 0.2f, 0.2f}, 0.1f),
            add_plastic(scn, "obj02", {0.2f, 0.5f, 0.2f}, 0.05f),
            add_plastic(scn, "obj03", {0.2f, 0.2f, 0.5f}, 0.01f)};
        add_objects(scn, mat);
    } else if (otype == "simple") {
        auto mat = std::vector<yobj::material*>{
            add_plastic(scn, "obj01", {1, 1, 1}, 0.1f,
                add_texture(scn, "rcolored.png")),
            add_plastic(scn, "obj02", {1, 1, 1}, 0.05f,
                add_texture(scn, "checker.png")),
            add_plastic(scn, "obj03", {1, 1, 1}, 0.01f,
                add_texture(scn, "colored.png"))};
        add_objects(scn, mat);
    } else if (otype == "transparent") {
        auto mat = std::vector<yobj::material*>{
            add_transparent_plastic(scn, "obj01", {1, 1, 1}, 0.1f, 0.1f,
                add_texture(scn, "rcolored.png")),
            add_transparent_plastic(scn, "obj02", {1, 1, 1}, 0.05f, 0.5f,
                add_texture(scn, "checker.png")),
            add_transparent_plastic(scn, "obj03", {1, 1, 1}, 0.01f, 0.9f,
                add_texture(scn, "colored.png"))};
        add_objects(scn, mat);
    } else if (otype == "transparentp") {
        auto mat = std::vector<yobj::material*>{
            add_transparent_plastic(scn, "obj01", {1, 1, 1}, 0.1f, 0.1f,
                add_texture(scn, "rcolored.png")),
            add_transparent_plastic(scn, "obj02", {1, 1, 1}, 0.05f, 0.5f,
                add_texture(scn, "checker.png")),
            add_transparent_plastic(scn, "obj03", {1, 1, 1}, 0.01f, 0.9f,
                add_texture(scn, "colored.png"))};
        add_instance(
            scn, "plane01", add_quad(scn, "plane01", mat[0], 2), {-2.5f, 0, 0});
        add_instance(
            scn, "plane02", add_quad(scn, "plane02", mat[1], 2), {0, 0, 0});
        add_instance(
            scn, "plane03", add_quad(scn, "plane03", mat[2], 2), {2.5f, 0, 0});
    } else if (otype == "refracted") {
        auto mat = std::vector<yobj::material*>{
            add_glass(scn, "obj01", {1, 1, 1}, 0.1f),
            add_glass(scn, "obj02", {1, 1, 1}, 0.05f),
            add_glass(scn, "obj03", {1, 1, 1}, 0.01f)};
        add_instance(
            scn, "obj01", add_quad(scn, "obj01", mat[0], 2), {-2.5f, 0, 0});
        add_instance(
            scn, "obj02", add_cube(scn, "obj02", mat[1], 2), {0, 0, 0});
        add_instance(
            scn, "obj03", add_sphere(scn, "obj03", mat[2], 5), {2.5f, 0, 0});
    } else if (otype == "refractedp") {
        auto mat = std::vector<yobj::material*>{
            add_glass(scn, "obj01", {1, 1, 1}, 0.05f),
            add_glass(scn, "obj02", {1, 1, 1}, 0.02f),
            add_glass(scn, "obj03", {1, 1, 1}, 0.01f)};
        add_instance(
            scn, "obj01", add_quad(scn, "obj01", mat[0], 2), {-2.5f, 0, 0});
        add_instance(
            scn, "obj02", add_quad(scn, "obj02", mat[1], 2), {0, 0, 0});
        add_instance(
            scn, "obj03", add_quad(scn, "obj03", mat[2], 2), {2.5f, 0, 0});
    } else if (otype == "points") {
        auto mat = add_diffuse(scn, "points", {0.2f, 0.2f, 0.2f});
        add_instance(
            scn, "points01", add_points(scn, "points01", mat, 64 * 64 * 16, 1));
    } else if (otype == "lines") {
        auto mat = add_diffuse(scn, "lines", {0.2f, 0.2f, 0.2f});
        auto imat = add_diffuse(scn, "interior", {0.2f, 0.2f, 0.2f});
        add_instance(scn, "lines_01",
            add_lines(scn, "lines01", mat, 64 * 64 * 16, 4, 0.1f, 0, 0),
            {-2.5f, 0, 0});
        add_instance(scn, "lines_interior_01",
            add_sphere(scn, "lines_interior_01", imat, 6), {-2.5f, 0, 0});
        add_instance(scn, "lines_02",
            add_lines(scn, "lines02", mat, 64 * 64 * 16, 4, 0, 0.75f, 0));
        add_instance(scn, "lines_interior_02",
            add_sphere(scn, "lines_interior_02", imat, 6));
        add_instance(scn, "lines_03",
            add_lines(scn, "lines03", mat, 64 * 64 * 16, 4, 0, 0, 0.5f),
            {2.5f, 0, 0});
        add_instance(scn, "lines_interior_03",
            add_sphere(scn, "lines_interior_03", imat, 6), {2.5f, 0, 0});
    } else if (otype == "hair") {
        auto mat = add_diffuse(scn, "lines", {0.2f, 0.2f, 0.2f});
        auto imat = add_diffuse(scn, "interior", {0.2f, 0.2f, 0.2f});
        auto objs = add_objects(scn, {imat, imat, imat});
        add_hair(scn, objs[0], ym::pow2(14), ym::pow2(2), 0.1f, 13, mat);
        add_hair(scn, objs[1], ym::pow2(14), ym::pow2(2), 0.1f, 17, mat);
        add_hair(scn, objs[2], ym::pow2(14), ym::pow2(2), 0.1f, 23, mat);
    } else if (otype == "sym_points01") {
        auto mat = add_material(scn, "sym_points");
        add_points_instance(
            scn, "points01", mat, 64 * 64 * 16, 1, {-2.5f, 0, 0});
        add_points_instance(scn, "points02", mat, 64 * 64 * 16, 1);
        add_points_instance(
            scn, "points03", mat, 64 * 64 * 16, 1, {2.5f, 0, 0});
    } else if (otype == "sym_points02") {
        auto mat = add_material(scn, "sym_points");
        auto omat = add_material(scn, "plastic01");
        add_objects(scn, {omat, omat, omat});
        add_points_instance(
            scn, "points01", mat, 64 * 64 * 16, 1, {-2.5f, 3, 0});
        add_points_instance(
            scn, "points02", mat, 64 * 64 * 16, 1, {0.0f, 3, 0});
        add_points_instance(
            scn, "points03", mat, 64 * 64 * 16, 1, {2.5f, 3, 0});
    } else if (otype == "sym_cloth01") {
        auto mat = add_material(scn, "sym_cloth");
        add_instance(scn, "sym_quad01", add_quad(scn, "sym_quad01", mat, 6),
            {-2.5f, 0, 0});
        add_instance(scn, "sym_quad02", add_quad(scn, "sym_quad02", mat, 6));
        add_instance(scn, "sym_quad03", add_quad(scn, "sym_quad03", mat, 6),
            {2.5f, 0, 0});
    } else if (otype == "sym_cloth02") {
        auto mat = add_material(scn, "sym_cloth");
        auto omat = add_material(scn, "plastic01");
        add_objects(scn, {omat, omat, omat});
        add_instance(scn, "sym_quad01", add_quad(scn, "sym_quad01", mat, 6),
            {-2.5f, 3, 0}, {-90, 0, 0});
        add_instance(scn, "sym_quad01", add_quad(scn, "sym_quad01", mat, 6),
            {0.0f, 3, 0.10f}, {-90, 0, 0});
        add_instance(scn, "sym_quad01", add_quad(scn, "sym_quad01", mat, 6),
            {2.5f, 3, 0.15f}, {-90, 0, 0});
    } else {
        throw std::runtime_error("bad value");
    }

    if (ltype == "pointlight") {
        auto mat = add_emission(scn, "pointlight", {400, 400, 400});
        add_instance(scn, "pointlight01", add_point(scn, "pointlight01", mat),
            {1.4f, 8, 6});
        add_instance(scn, "pointlight02", add_point(scn, "pointlight02", mat),
            {-1.4f, 8, 6});
    } else if (ltype == "arealight") {
        auto mat = add_emission(scn, "arealight", {40, 40, 40});
        auto shp = add_quad(scn, "arealight", mat, 0, 2);
        add_instance(scn, "arealight01", shp, {-4, 4, 8},
            make_lookat_frame({-4, 4, 8}, {0, 2, 0}).rot());
        add_instance(scn, "arealight02", shp, {4, 4, 8},
            make_lookat_frame({4, 4, 8}, {0, 2, 0}).rot());
    } else if (ltype == "envlight") {
        auto mat = add_emission(
            scn, "envlight", {1, 1, 1}, add_texture(scn, "env.hdr"));
        add_env(scn, "envlight", mat);
    } else {
        throw std::runtime_error("bad value");
    }

    return scn;
}

yobj::scene* make_mesh_scene(const std::string& otype) {
    auto scn = new yobj::scene();

    if (otype == "cube") {
        add_cube(scn, otype, add_material(scn, "plastic00"), 0);
    } else if (otype == "sphere") {
        add_sphere(scn, otype, add_material(scn, "plastic00"), 4);
    } else {
        throw std::runtime_error("bad value");
    }

    return scn;
}

yobj::scene* make_rigid_scene(
    const std::string& otype, const std::string& ltype) {
    auto scn = new yobj::scene();
    add_camera(scn, "cam", {10, 10, 10}, {0, 0, 0}, 0.5f, 0);
    add_camera(scn, "cam_dof", {10, 10, 10}, {0, 0, 0}, 0.5f, 0.1f);

    if (otype == "flat") {
        auto fmat = add_diffuse(scn, "floor", {1, 1, 1});
        auto mat = add_plastic(
            scn, "obj", {1, 1, 1}, 0.1f, add_texture(scn, "checker.png"));
        add_instance(scn, "floor", add_box(scn, "floor", fmat, 6, {12, 1, 12}),
            {0, -1, 0});
        add_instance(
            scn, "obj01", add_cube(scn, "obj01", mat, 4), {-2.5f, 0.5f, 0});
        add_instance(
            scn, "obj02", add_spherecube(scn, "obj02", mat, 4), {0, 1, 0});
        add_instance(
            scn, "obj03", add_cube(scn, "obj03", mat, 4), {2.5f, 1.5f, 0});
        add_instance(
            scn, "obj11", add_cube(scn, "obj11", mat, 4), {-2.5f, 0.5f, 3});
        add_instance(
            scn, "obj12", add_spherecube(scn, "obj12", mat, 4), {0, 1.0f, 3});
        add_instance(
            scn, "obj13", add_cube(scn, "obj13", mat, 4), {2.5f, 1.5f, 3});
        add_instance(
            scn, "obj21", add_cube(scn, "obj21", mat, 4), {-2.5f, 0.5f, -3});
        add_instance(
            scn, "obj22", add_spherecube(scn, "obj22", mat, 4), {0, 1.0f, -3});
        add_instance(
            scn, "obj23", add_cube(scn, "obj23", mat, 4), {2.5f, 1.5f, -3});
    } else if (otype == "slanted") {
        auto fmat = add_diffuse(scn, "floor", {1, 1, 1});
        auto mat = add_plastic(
            scn, "obj", {1, 1, 1}, 0.1f, add_texture(scn, "checker.png"));
        add_instance(scn, "floor", add_box(scn, "floor", fmat, 6, {12, 1, 12}),
            {0, -3, 0}, {30, 0, 0});
        add_instance(scn, "obj01", add_cube(scn, "obj01", mat, 4),
            {-2.5f, 1.0f, 0}, {0, 0, 45});
        add_instance(
            scn, "obj02", add_spherecube(scn, "obj02", mat, 4), {0, 1, 0});
        add_instance(
            scn, "obj03", add_cube(scn, "obj03", mat, 4), {2.5f, 1, 0});
        add_instance(scn, "obj11", add_cube(scn, "obj11", mat, 4),
            {-2.5f, 1, 3}, {0, 0, 45});
        add_instance(
            scn, "obj12", add_spherecube(scn, "obj12", mat, 4), {0, 1, 3});
        add_instance(
            scn, "obj13", add_cube(scn, "obj13", mat, 4), {2.5f, 1, 3});
        add_instance(scn, "obj21", add_cube(scn, "obj21", mat, 4),
            {-2.5f, 2.5f, -3}, {0, 0, 45});
        add_instance(
            scn, "obj22", add_spherecube(scn, "obj22", mat, 4), {0, 2.5f, -3});
        add_instance(
            scn, "obj23", add_cube(scn, "obj23", mat, 4), {2.5f, 2.5f, -3});
    } else {
        throw std::runtime_error("bad value");
    }

    if (ltype == "pointlight") {
        auto mat = add_emission(scn, "pointlight", {400, 400, 400});
        add_instance(scn, "pointlight01", add_point(scn, "pointlight01", mat),
            {1.4f, 8, 6});
        add_instance(scn, "pointlight02", add_point(scn, "pointlight01", mat),
            {-1.4f, 8, 6});
    } else {
        throw std::runtime_error("bad value");
    }

    return scn;
}

template <typename T>
static inline int index(const std::vector<T*>& vec, T* val) {
    auto pos = std::find(vec.begin(), vec.end(), val);
    if (pos != vec.end()) return (int)(pos - vec.begin());
    return -1;
}

ygltf::scene_group* obj2gltf(const yobj::scene* obj, bool add_scene) {
    auto gltf = new ygltf::scene_group();

    // convert textures
    for (auto otxt : obj->textures) {
        auto gtxt = new ygltf::texture();
        gtxt->path = otxt->path;
        gtxt->ldr = otxt->ldr;
        gtxt->hdr = otxt->hdr;
        gltf->textures.push_back(gtxt);
    }

    // convert materials
    for (auto omat : obj->materials) {
        auto gmat = new ygltf::material();
        gmat->name = omat->name;
        gmat->emission = omat->ke;
        gmat->emission_txt =
            (index(obj->textures, omat->ke_txt) < 0) ?
                nullptr :
                gltf->textures[index(obj->textures, omat->ke_txt)];
        if (!omat->unknown_props.at("PBR_HACK").empty()) {
            auto mstr = omat->unknown_props.at("PBR_HACK").at(0);
            if (mstr == "emission") {
            } else if (mstr == "diffuse") {
                gmat->metallic_roughness =
                    new ygltf::material_metallic_rooughness();
                auto gmr = gmat->metallic_roughness;
                gmr->base = omat->kd;
                gmr->opacity = omat->opacity;
                gmr->metallic = 0;
                gmr->roughness = 1;
                gmr->base_txt =
                    (index(obj->textures, omat->kd_txt) < 0) ?
                        nullptr :
                        gltf->textures[index(obj->textures, omat->kd_txt)];
            } else if (mstr == "plastic") {
                gmat->metallic_roughness =
                    new ygltf::material_metallic_rooughness();
                auto gmr = gmat->metallic_roughness;
                gmr->base = omat->kd;
                gmr->opacity = omat->opacity;
                gmr->metallic = 0;
                gmr->roughness = sqrtf(omat->rs);
                gmr->base_txt =
                    (index(obj->textures, omat->kd_txt) < 0) ?
                        nullptr :
                        gltf->textures[index(obj->textures, omat->kd_txt)];
            } else if (mstr == "metal") {
                gmat->metallic_roughness =
                    new ygltf::material_metallic_rooughness();
                auto gmr = gmat->metallic_roughness;
                gmr->base = omat->ks;
                gmr->opacity = omat->opacity;
                gmr->metallic = 1;
                gmr->roughness = sqrtf(omat->rs);
                gmr->base_txt =
                    (index(obj->textures, omat->ks_txt) < 0) ?
                        nullptr :
                        gltf->textures[index(obj->textures, omat->ks_txt)];
            } else if (mstr == "glass") {
            } else {
                throw std::exception();
            }
        } else {
            gmat->metallic_roughness =
                new ygltf::material_metallic_rooughness();
            auto gmr = gmat->metallic_roughness;
            gmr->base = omat->kd;
            gmr->opacity = omat->opacity;
            gmr->metallic = 0;
            gmr->roughness = sqrtf(omat->rs);
            gmr->base_txt =
                (index(obj->textures, omat->kd_txt) < 0) ?
                    nullptr :
                    gltf->textures[index(obj->textures, omat->kd_txt)];
        }
        gltf->materials.push_back(gmat);
    }

    // convert meshes
    for (auto omesh : obj->meshes) {
        auto gmesh = new ygltf::mesh();
        gmesh->name = omesh->name;
        for (auto oprim : omesh->shapes) {
            auto gprim = new ygltf::shape();
            gprim->mat = (index(obj->materials, oprim->mat) < 0) ?
                             nullptr :
                             gltf->materials[index(obj->materials, oprim->mat)];
            gprim->pos = oprim->pos;
            gprim->norm = oprim->norm;
            gprim->texcoord = oprim->texcoord;
            gprim->color = oprim->color;
            gprim->points = oprim->points;
            gprim->lines = oprim->lines;
            gprim->triangles = oprim->triangles;
            gmesh->shapes.push_back(gprim);
        }
        gltf->meshes.push_back(gmesh);
    }

    if (add_scene) {
        // init nodes
        auto scn = new ygltf::scene();
        scn->name = "scene";
        gltf->default_scene = scn;
        gltf->scenes.push_back(scn);

        // convert instances
        if (obj->instances.empty()) {
            for (auto msh : gltf->meshes) {
                auto gnode = new ygltf::node();
                gnode->name = msh->name;
                gnode->msh = msh;
                scn->nodes.push_back(gnode);
                gltf->nodes.push_back(gnode);
            }
        } else {
            for (auto oist : obj->instances) {
                auto gnode = new ygltf::node();
                gnode->name = oist->name;
                gnode->translation = oist->translation;
                gnode->rotation = oist->rotation;
                gnode->scale = oist->scale;
                gnode->matrix = oist->matrix;
                gnode->msh = gltf->meshes[index(obj->meshes, oist->msh)];
                scn->nodes.push_back(gnode);
                gltf->nodes.push_back(gnode);
            }
        }

        // convert cameras
        if (obj->cameras.empty()) {
            ygltf::add_default_cameras(gltf);
        } else {
            // TODO: convert cameras
            for (auto ocam : obj->cameras) {
                auto gcam = new ygltf::camera();
                gcam->name = ocam->name;
                gcam->ortho = ocam->ortho;
                gcam->yfov = ocam->yfov;
                gcam->aspect = ocam->aspect;
                gcam->focus = ocam->focus;
                gcam->aperture = ocam->aperture;
                gcam->near = 0.1f;
                gcam->far = 10000.f;
                gltf->cameras.push_back(gcam);
                auto gnode = new ygltf::node();
                gnode->name = ocam->name;
                gnode->matrix = ocam->matrix;
                gnode->cam = gcam;
                scn->nodes.push_back(gnode);
                gltf->nodes.push_back(gnode);
            }
        }
    }

    // done
    return gltf;
}

void save_scene(const std::string& scenename, const std::string& dirname,
    yobj::scene* oscn, bool add_gltf_scene = true) {
    auto gscn = obj2gltf(oscn, add_gltf_scene);

    if (!yu::string::startswith(scenename, "instance"))
        yobj::flatten_instances(oscn);

    yobj::save_scene(dirname + "/" + scenename + ".obj", oscn, false);
    ygltf::save_scenes(
        dirname + "/" + scenename + ".gltf", scenename + ".bin", gscn, false);

    delete oscn;
    delete gscn;
}

int main(int argc, char* argv[]) {
    // mesh-only scenes -------------------------
    auto motypes = std::vector<std::string>{
        "cube", "sphere",
    };

    // simple scenes ----------------------------
    auto stypes = std::vector<std::string>{"basic", "simple", "transparent",
        "transparentp", "refracted", "refractedp", "lines", "points", "hair",
        "sym_points01", "sym_points02", "sym_cloth01", "sym_cloth02"};

    // matball scenes --------------------------
    auto mtypes = std::vector<std::string>{"matte00", "matte01_txt",
        "plastic01", "plastic02", "plastic03", "plastic04", "plastic01_txt",
        "plastic02_txt", "plastic03_txt", "plastic04_txt", "gold01", "gold02",
        "copper01", "copper02", "silver01", "silver02", "bump00", "bump01",
        "bump02", "bump03"};

    // instance scenes -------------------------
    auto itypes = std::vector<std::string>{
        "instance100", "instance1600", "instance2500", "instance10000"};

    // put together scene names
    auto scene_names = std::vector<std::string>{"all"};
    for (auto stype : stypes) scene_names += stype;
    for (auto itype : itypes) scene_names += itype;
    for (auto mtype : mtypes) scene_names += "matball_" + mtype;
    for (auto motype : motypes) scene_names += "mesh_" + motype;
    scene_names += {"cornell_box", "rigid", "textures"};

    // command line params
    auto parser =
        yu::cmdline::make_parser(argc, argv, "ytestgen", "make tests");
    auto scene = parse_opts(
        parser, "--scene", "-s", "scene name", "all", false, scene_names);
    auto dirname =
        parse_opts(parser, "--dirname", "-d", "directory name", "tests");
    auto no_parallel =
        parse_flag(parser, "--no-parallel", "", "do not run in parallel");
    check_parser(parser);

    // run lambda (allows for multi- and single- threaded switch)
    auto run_async = [no_parallel](const std::function<void(void)>& func) {
        if (!no_parallel) {
            yu::concurrent::run_async(func);
        } else {
            func();
        }
    };

// make directories
#ifndef _MSC_VER
    auto rcmd = "rm -rf " + dirname;
    system(rcmd.c_str());
    auto cmd = "mkdir " + dirname;
    system(cmd.c_str());
#else
    auto rcmd = "del " + dirname + "\\*.*; rmdir " + dirname;
    system(rcmd.c_str());
    auto cmd = "mkdir " + dirname;
    system(cmd.c_str());
#endif

    // meshes scene ------------------------------
    for (auto motype : motypes) {
        auto sname = "mesh_" + motype;
        if (scene != "all" && scene != sname) continue;
        run_async([=] {
            printf("generating %s scenes ...\n", sname.c_str());
            auto scn = make_mesh_scene(motype);
            save_scene(sname, dirname, scn, false);
        });
    }

    // simple scene ------------------------------
    for (auto stype : stypes) {
        if (scene != "all" && scene != stype) continue;
        run_async([=] {
            printf("generating %s scenes ...\n", stype.c_str());
            for (auto ltype : {"pointlight", "arealight", "envlight"}) {
                auto scn = make_simple_scene(stype, ltype);
                save_scene(stype + "_" + std::string(ltype), dirname, scn);
            }
        });
    }

    // instance scenes --------------------------
    for (auto itype : itypes) {
        if (scene != "all" && scene != itype) continue;
        run_async([=] {
            printf("generating %s scenes ...\n", itype.c_str());
            for (auto ltype : {"pointlight", "arealight", "envlight"}) {
                auto scn = make_instance_scene(itype, ltype);
                save_scene(itype + "_" + std::string(ltype), dirname, scn);
            }
        });
    }

    // matball scene --------------------------
    auto mbtype = "matball01";
    for (auto mtype : mtypes) {
        auto sname = "matball_" + mtype;
        if (scene != "all" && scene != sname) continue;
        run_async([=] {
            printf("generating %s scenes ...\n", sname.c_str());
            for (auto ltype : {"pointlight", "arealight", "envlight"}) {
                auto scn = make_matball_scene(mbtype, mtype, ltype);
                save_scene(sname + "_" + std::string(ltype), dirname, scn);
            }
        });
    }

    // cornell box ------------------------------
    if (scene == "cornell_box" || scene == "all") {
        run_async([=] {
            printf("generating cornell box scenes ...\n");
            auto scn = make_cornell_box_scene();
            save_scene("cornell_box", dirname, scn);
        });
    }

    // rigid body scenes ------------------------
    if (scene == "rigid" || scene == "all") {
        run_async([=] {
            printf("generating rigid body scenes ...\n");
            for (auto otype : {"flat", "slanted"}) {
                auto ltype = "pointlight";
                auto scn = make_rigid_scene(otype, ltype);
                save_scene(
                    "rigid_" + std::string(otype) + "_" + std::string(ltype),
                    dirname, scn);
            }
        });
    }

    // textures ---------------------------------
    if (scene == "textures" || scene == "all") {
        run_async([=] {
            printf("generating simple textures ...\n");
            save_image("grid.png", dirname, ym::make_grid_image(512));
            save_image("checker.png", dirname, ym::make_checker_image(512));
            save_image("rchecker.png", dirname,
                ym::make_recuvgrid_image(512, 64, false));
            save_image("colored.png", dirname, ym::make_uvgrid_image(512));
            save_image("rcolored.png", dirname, ym::make_recuvgrid_image(512));
            save_image("gamma.png", dirname, ym::make_gammaramp_image(512));
            save_image("grid_normal.png", dirname,
                ym::bump_to_normal_map(ym::make_grid_image(512), 4));
            save_image("checker_normal.png", dirname,
                ym::bump_to_normal_map(ym::make_checker_image(512), 4));
            save_image("gamma.hdr", dirname, ym::make_gammaramp_imagef(512));
            printf("generating envmaps textures ...\n");
            save_image("env.hdr", dirname,
                make_sunsky_hdr(1024, 512, 0.8f, 8, ym::vec3f{0.2f, 0.2f, 0.2f},
                    1 / powf(2, 6), true));
            save_image("env01.hdr", dirname,
                make_sunsky_hdr(1024, 512, 0.8f, 8, ym::vec3f{0.2f, 0.2f, 0.2f},
                    1 / powf(2, 6), true));
        });
    }

    // waiting for all tasks to complete
    if (!no_parallel) yu::concurrent::wait_pool();
}
