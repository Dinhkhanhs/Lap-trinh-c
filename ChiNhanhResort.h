// ============================================================
//  PHẦN D.1 - ChiNhanhResort
// ============================================================
#ifndef CHINHANH_RESORT_H
#define CHINHANH_RESORT_H

#include <memory>
#include <string>
#include <vector>

#include "Phong.h"
#include "NhanVien.h"
#include "PhieuDatPhong.h"

// Tên trạng thái trả về từ Phong::layTenTrangThai().
// Chỉnh lại cho khớp với chuỗi mà các lớp Concrete State (phần A) trả về.
namespace TenTrangThai {
    inline const std::string TRONG    = "Trong";
    inline const std::string GIU_CHO  = "Giu cho";
    inline const std::string DANG_O   = "Dang o";
    inline const std::string DANG_DON = "Dang don";
    inline const std::string BAO_TRI  = "Bao tri";
}

// Hai khoảng ngày [a1, a2) và [b1, b2) có giao nhau không?
// Ngày theo định dạng "yyyy-mm-dd" nên so sánh chuỗi chính là so sánh thời gian.
inline bool trungKhoangNgay(const std::string& a1, const std::string& a2,
                            const std::string& b1, const std::string& b2) {
    return a1 < b2 && b1 < a2;
}

class ChiNhanhResort {
private:
    std::string maChiNhanh;
    std::string tenChiNhanh;
    std::string diaChi;
    std::vector<std::shared_ptr<Phong>>    danhSachPhong;
    std::vector<std::shared_ptr<NhanVien>> danhSachNhanVien;

public:
    ChiNhanhResort() = default;
    ChiNhanhResort(const std::string& ma, const std::string& ten, const std::string& diaChi);
    virtual ~ChiNhanhResort() = default;

    // --- Quản lý phòng ---
    void themPhong(std::shared_ptr<Phong> phong);          // ném invalid_argument nếu null/trùng mã
    bool xoaPhong(const std::string& maPhong);
    std::shared_ptr<Phong> timPhong(const std::string& maPhong) const;

    // Phòng có thể đặt trong [checkIn, checkOut). Nếu truyền danh sách phiếu đặt
    // thì loại thêm các phòng đã bị đặt trùng lịch.
    std::vector<std::shared_ptr<Phong>> layPhongTrong(
        const std::string& checkIn, const std::string& checkOut,
        const std::vector<std::shared_ptr<PhieuDatPhong>>& dsPhieu = {}) const;

    // --- Quản lý nhân viên ---
    void themNhanVien(std::shared_ptr<NhanVien> nv);

    // Công suất sử dụng (%) = số phòng đang ở / tổng số phòng * 100
    double tinhCongSuatSuDung() const;

    // --- Getter ---
    const std::string& layMaChiNhanh()  const { return maChiNhanh; }
    const std::string& layTenChiNhanh() const { return tenChiNhanh; }
    const std::string& layDiaChi()      const { return diaChi; }
    const std::vector<std::shared_ptr<Phong>>&    layDanhSachPhong()    const { return danhSachPhong; }
    const std::vector<std::shared_ptr<NhanVien>>& layDanhSachNhanVien() const { return danhSachNhanVien; }
};

#endif
