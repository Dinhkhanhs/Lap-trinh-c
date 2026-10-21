// ============================================================
//  PHẦN D.2 - QuanLyKhachSan (Core Manager)
// ============================================================
#ifndef QUAN_LY_KHACH_SAN_H
#define QUAN_LY_KHACH_SAN_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ChiNhanhResort.h"
#include "KhachHang.h"
#include "PhieuDatPhong.h"
#include "HoaDon.h"
#include "DichVu.h"

class QuanLyKhachSan {
private:
    std::vector<std::shared_ptr<ChiNhanhResort>> danhSachChiNhanh;
    std::vector<std::shared_ptr<KhachHang>>      danhSachKhachHang;
    std::vector<std::shared_ptr<PhieuDatPhong>>  danhSachPhieuDat;
    std::vector<std::shared_ptr<HoaDon>>         danhSachHoaDon;
    std::vector<std::shared_ptr<DichVu>>         danhSachDichVu;

    // Dịch vụ phát sinh theo từng phòng (maPhong -> các dịch vụ), dùng khi lập hóa đơn lúc check-out
    std::map<std::string, std::vector<std::shared_ptr<DichVu>>> dichVuTheoPhong;

    int demPhieu  = 0;   // sinh mã PDxxxx
    int demHoaDon = 0;   // sinh mã HDxxxx

    // --- Hàm nội bộ ---
    bool kiemTraPhongKhaDung(const std::shared_ptr<Phong>& phong,
                             const std::string& checkIn, const std::string& checkOut) const;
    std::shared_ptr<PhieuDatPhong> timPhieu(const std::string& maPhieu) const;
    std::shared_ptr<Phong>         timPhongToanHeThong(const std::string& maPhong) const;
    bool coPhieuConHieuLuc(const std::string& maPhong) const;

    static bool        ngayHopLe(const std::string& ngay);               // yyyy-mm-dd
    static std::string homNay();
    static MuaDuLich   xacDinhMua(const std::string& ngay);
    static std::string taoMa(const std::string& tienTo, int so);

public:
    QuanLyKhachSan() = default;
    virtual ~QuanLyKhachSan() = default;

    // --- Chi nhánh / khách hàng / dịch vụ ---
    void themChiNhanh(std::shared_ptr<ChiNhanhResort> cn);
    void themKhachHang(std::shared_ptr<KhachHang> kh);
    std::shared_ptr<KhachHang> timKhachHang(const std::string& ma) const;
    void themDichVu(std::shared_ptr<DichVu> dv);                          // thêm vào danh mục

    // --- Nghiệp vụ đặt phòng ---
    std::shared_ptr<PhieuDatPhong> datPhong(std::shared_ptr<KhachHang> kh, std::shared_ptr<Phong> phong,
                                            const std::string& checkIn, const std::string& checkOut);
    bool checkIn(const std::string& maPhieu);
    std::shared_ptr<HoaDon> checkOut(const std::string& maPhieu);
    bool huyDatPhong(const std::string& maPhieu);
    void themDichVuChoPhong(const std::string& maPhong, std::shared_ptr<DichVu> dv);

    // --- Thống kê ---
    double thongKeDoanhThu(int thang, int nam) const;
    double thongKeCongSuat() const;     // (phòng đang ở / tổng phòng) * 100%

    // --- File ---
    void luuFile(const std::string& tenFile) const;
    void docFile(const std::string& tenFile);

    // --- Getter phục vụ module UI ---
    const std::vector<std::shared_ptr<ChiNhanhResort>>& layDanhSachChiNhanh() const { return danhSachChiNhanh; }
    const std::vector<std::shared_ptr<KhachHang>>&      layDanhSachKhachHang() const { return danhSachKhachHang; }
    const std::vector<std::shared_ptr<PhieuDatPhong>>&  layDanhSachPhieuDat() const { return danhSachPhieuDat; }
    const std::vector<std::shared_ptr<HoaDon>>&         layDanhSachHoaDon() const { return danhSachHoaDon; }
    const std::vector<std::shared_ptr<DichVu>>&         layDanhSachDichVu() const { return danhSachDichVu; }
};

#endif
