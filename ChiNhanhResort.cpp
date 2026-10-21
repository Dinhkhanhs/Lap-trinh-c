#include "ChiNhanhResort.h"

#include <algorithm>
#include <stdexcept>

ChiNhanhResort::ChiNhanhResort(const std::string& ma, const std::string& ten, const std::string& diaChi)
    : maChiNhanh(ma), tenChiNhanh(ten), diaChi(diaChi) {}

void ChiNhanhResort::themPhong(std::shared_ptr<Phong> phong) {
    if (!phong)
        throw std::invalid_argument("Phong khong hop le (nullptr)");
    if (timPhong(phong->layMaPhong()))
        throw std::invalid_argument("Ma phong da ton tai: " + phong->layMaPhong());
    danhSachPhong.push_back(std::move(phong));
}

bool ChiNhanhResort::xoaPhong(const std::string& maPhong) {
    auto it = std::find_if(danhSachPhong.begin(), danhSachPhong.end(),
        [&](const std::shared_ptr<Phong>& p) { return p->layMaPhong() == maPhong; });
    if (it == danhSachPhong.end()) return false;

    // Không cho xoá phòng đang có khách ở hoặc đang được giữ chỗ
    const std::string tt = (*it)->layTenTrangThai();
    if (tt == TenTrangThai::DANG_O || tt == TenTrangThai::GIU_CHO) return false;

    danhSachPhong.erase(it);
    return true;
}

std::shared_ptr<Phong> ChiNhanhResort::timPhong(const std::string& maPhong) const {
    for (const auto& p : danhSachPhong)
        if (p->layMaPhong() == maPhong) return p;
    return nullptr;
}

std::vector<std::shared_ptr<Phong>> ChiNhanhResort::layPhongTrong(
    const std::string& checkIn, const std::string& checkOut,
    const std::vector<std::shared_ptr<PhieuDatPhong>>& dsPhieu) const {

    std::vector<std::shared_ptr<Phong>> ketQua;
    if (!(checkIn < checkOut)) return ketQua;   // khoảng ngày không hợp lệ

    for (const auto& p : danhSachPhong) {
        if (p->layTenTrangThai() == TenTrangThai::BAO_TRI) continue;

        bool biTrung = false;
        for (const auto& ph : dsPhieu) {
            if (!ph || ph->kiemTraDaHuy()) continue;
            if (ph->layPhong()->layMaPhong() != p->layMaPhong()) continue;
            if (trungKhoangNgay(checkIn, checkOut, ph->layNgayCheckIn(), ph->layNgayCheckOut())) {
                biTrung = true;
                break;
            }
        }
        if (!biTrung) ketQua.push_back(p);
    }
    return ketQua;
}

void ChiNhanhResort::themNhanVien(std::shared_ptr<NhanVien> nv) {
    if (!nv)
        throw std::invalid_argument("Nhan vien khong hop le (nullptr)");
    for (const auto& x : danhSachNhanVien)
        if (x->layMaSo() == nv->layMaSo())
            throw std::invalid_argument("Ma nhan vien da ton tai: " + nv->layMaSo());
    danhSachNhanVien.push_back(std::move(nv));
}

double ChiNhanhResort::tinhCongSuatSuDung() const {
    if (danhSachPhong.empty()) return 0.0;
    int dangO = 0;
    for (const auto& p : danhSachPhong)
        if (p->layTenTrangThai() == TenTrangThai::DANG_O) ++dangO;
    return static_cast<double>(dangO) / danhSachPhong.size() * 100.0;
}
