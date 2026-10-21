#include "QuanLyKhachSan.h"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

#include "StandardRoom.h"
#include "DeluxeRoom.h"
#include "Suite.h"
#include "Villa.h"

// ------------------------------------------------------------
//  Hàm tiện ích
// ------------------------------------------------------------
bool QuanLyKhachSan::ngayHopLe(const std::string& ngay) {
    int y, m, d;
    char tail;
    if (std::sscanf(ngay.c_str(), "%4d-%2d-%2d%c", &y, &m, &d, &tail) != 3) return false;
    if (ngay.size() != 10 || y < 1900 || m < 1 || m > 12 || d < 1) return false;
    static const int ngayTrongThang[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int max = ngayTrongThang[m - 1];
    if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)) max = 29;
    return d <= max;
}

std::string QuanLyKhachSan::homNay() {
    std::time_t t = std::time(nullptr);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&t));
    return buf;
}

MuaDuLich QuanLyKhachSan::xacDinhMua(const std::string& ngay) {
    int y = 0, m = 0, d = 0;
    std::sscanf(ngay.c_str(), "%d-%d-%d", &y, &m, &d);
    if (m == 1 || m == 2)  return MuaDuLich::LE_TET;
    if (m >= 5 && m <= 8)  return MuaDuLich::CAO_DIEM;
    if (m >= 9 && m <= 11) return MuaDuLich::THAP_DIEM;
    return MuaDuLich::BINH_THUONG;
}

std::string QuanLyKhachSan::taoMa(const std::string& tienTo, int so) {
    std::ostringstream os;
    os << tienTo << std::setw(4) << std::setfill('0') << so;
    return os.str();
}

std::shared_ptr<PhieuDatPhong> QuanLyKhachSan::timPhieu(const std::string& maPhieu) const {
    for (const auto& p : danhSachPhieuDat)
        if (p->layMaPhieu() == maPhieu) return p;
    return nullptr;
}

std::shared_ptr<Phong> QuanLyKhachSan::timPhongToanHeThong(const std::string& maPhong) const {
    for (const auto& cn : danhSachChiNhanh)
        if (auto p = cn->timPhong(maPhong)) return p;
    return nullptr;
}

bool QuanLyKhachSan::coPhieuConHieuLuc(const std::string& maPhong) const {
    for (const auto& ph : danhSachPhieuDat)
        if (!ph->kiemTraDaHuy() && ph->layPhong()->layMaPhong() == maPhong) return true;
    return false;
}

bool QuanLyKhachSan::kiemTraPhongKhaDung(const std::shared_ptr<Phong>& phong,
                                         const std::string& checkIn, const std::string& checkOut) const {
    if (!phong) return false;
    if (phong->layTenTrangThai() == TenTrangThai::BAO_TRI) return false;
    for (const auto& ph : danhSachPhieuDat) {
        if (ph->kiemTraDaHuy()) continue;
        if (ph->layPhong()->layMaPhong() != phong->layMaPhong()) continue;
        if (trungKhoangNgay(checkIn, checkOut, ph->layNgayCheckIn(), ph->layNgayCheckOut()))
            return false;
    }
    return true;
}

// ------------------------------------------------------------
//  Chi nhánh / khách hàng / dịch vụ
// ------------------------------------------------------------
void QuanLyKhachSan::themChiNhanh(std::shared_ptr<ChiNhanhResort> cn) {
    if (!cn) throw std::invalid_argument("Chi nhanh khong hop le (nullptr)");
    for (const auto& x : danhSachChiNhanh)
        if (x->layMaChiNhanh() == cn->layMaChiNhanh())
            throw std::invalid_argument("Ma chi nhanh da ton tai: " + cn->layMaChiNhanh());
    danhSachChiNhanh.push_back(std::move(cn));
}

void QuanLyKhachSan::themKhachHang(std::shared_ptr<KhachHang> kh) {
    if (!kh) throw std::invalid_argument("Khach hang khong hop le (nullptr)");
    if (timKhachHang(kh->layMaSo()))
        throw std::invalid_argument("Ma khach hang da ton tai: " + kh->layMaSo());
    danhSachKhachHang.push_back(std::move(kh));
}

std::shared_ptr<KhachHang> QuanLyKhachSan::timKhachHang(const std::string& ma) const {
    for (const auto& kh : danhSachKhachHang)
        if (kh->layMaSo() == ma) return kh;
    return nullptr;
}

void QuanLyKhachSan::themDichVu(std::shared_ptr<DichVu> dv) {
    if (!dv) throw std::invalid_argument("Dich vu khong hop le (nullptr)");
    for (const auto& x : danhSachDichVu)
        if (x->layMaDichVu() == dv->layMaDichVu()) return;   // đã có trong danh mục
    danhSachDichVu.push_back(std::move(dv));
}

// ------------------------------------------------------------
//  Đặt phòng
// ------------------------------------------------------------
std::shared_ptr<PhieuDatPhong> QuanLyKhachSan::datPhong(
    std::shared_ptr<KhachHang> kh, std::shared_ptr<Phong> phong,
    const std::string& checkIn, const std::string& checkOut) {

    if (!kh || !phong)
        throw std::invalid_argument("Khach hang hoac phong khong hop le");
    if (!ngayHopLe(checkIn) || !ngayHopLe(checkOut) || !(checkIn < checkOut))
        throw std::invalid_argument("Ngay check-in/check-out khong hop le (yyyy-mm-dd, in < out)");
    if (!kiemTraPhongKhaDung(phong, checkIn, checkOut))
        return nullptr;   // phòng bảo trì hoặc trùng lịch

    auto phieu = std::make_shared<PhieuDatPhong>(
        taoMa("PD", ++demPhieu), kh, phong,
        /*nhanVienLap*/ nullptr, homNay(), checkIn, checkOut,
        /*soNguoi*/ 1, /*tienCoc*/ 0.0, xacDinhMua(checkIn));

    if (!phieu->xacNhanDatPhong()) {
        --demPhieu;
        return nullptr;
    }

    danhSachPhieuDat.push_back(phieu);
    kh->themLichSu(phieu);

    // Giữ chỗ phòng. Nếu phòng đang ở trạng thái không cho giữ chỗ
    // (vd đang có khách ở kỳ khác) thì bỏ qua, lịch đã được kiểm tra trùng ở trên.
    try { phong->giuCho(); } catch (const std::logic_error&) {}

    return phieu;
}

bool QuanLyKhachSan::checkIn(const std::string& maPhieu) {
    auto phieu = timPhieu(maPhieu);
    if (!phieu || phieu->kiemTraDaHuy()) return false;

    try {
        if (!phieu->checkIn()) return false;
        phieu->layPhong()->nhanPhong();
    } catch (const std::logic_error&) {
        return false;   // trạng thái phòng không cho nhận phòng
    }
    return true;
}

std::shared_ptr<HoaDon> QuanLyKhachSan::checkOut(const std::string& maPhieu) {
    auto phieu = timPhieu(maPhieu);
    if (!phieu || phieu->kiemTraDaHuy()) return nullptr;

    auto phong = phieu->layPhong();
    try {
        if (!phieu->checkOut()) return nullptr;
        phong->traPhong();
        phong->donPhong();        // sau khi trả phòng chuyển sang dọn phòng
    } catch (const std::logic_error&) {
        return nullptr;
    }

    // Lập hóa đơn
    auto hd = std::make_shared<HoaDon>(taoMa("HD", ++demHoaDon), phieu);

    // Gom dịch vụ phát sinh của phòng (cùng dịch vụ xuất hiện nhiều lần -> tăng số lượng)
    auto it = dichVuTheoPhong.find(phong->layMaPhong());
    if (it != dichVuTheoPhong.end()) {
        std::map<std::shared_ptr<DichVu>, int> dem;
        for (const auto& dv : it->second) ++dem[dv];
        for (const auto& kv : dem) hd->themDichVu(kv.first, kv.second);
        dichVuTheoPhong.erase(it);
    }
    danhSachHoaDon.push_back(hd);

    // Tích điểm: 1 điểm / 100.000 VND, rồi cập nhật hạng
    if (auto kh = phieu->layKhachHang()) {
        kh->congDiemTichLuy(static_cast<int>(hd->tinhTongTien() / 100000.0));
        kh->nangHang();
    }
    return hd;
}

bool QuanLyKhachSan::huyDatPhong(const std::string& maPhieu) {
    auto phieu = timPhieu(maPhieu);
    if (!phieu || phieu->kiemTraDaHuy()) return false;

    phieu->huyPhieu();

    // Nếu phòng không còn phiếu hiệu lực nào khác thì trả phòng về trạng thái ban đầu
    auto phong = phieu->layPhong();
    if (phong && !coPhieuConHieuLuc(phong->layMaPhong())) {
        try { phong->hoanTat(); } catch (const std::logic_error&) {}
    }
    return true;
}

void QuanLyKhachSan::themDichVuChoPhong(const std::string& maPhong, std::shared_ptr<DichVu> dv) {
    if (!dv) throw std::invalid_argument("Dich vu khong hop le (nullptr)");
    auto phong = timPhongToanHeThong(maPhong);
    if (!phong) throw std::invalid_argument("Khong tim thay phong: " + maPhong);
    if (phong->layTenTrangThai() != TenTrangThai::DANG_O)
        throw std::logic_error("Chi them dich vu cho phong dang co khach o");

    themDichVu(dv);                       // đảm bảo có trong danh mục
    dichVuTheoPhong[maPhong].push_back(dv);
}

// ------------------------------------------------------------
//  Thống kê
// ------------------------------------------------------------
double QuanLyKhachSan::thongKeDoanhThu(int thang, int nam) const {
    double tong = 0.0;
    for (const auto& hd : danhSachHoaDon) {
        if (!hd->kiemTraDaThanhToan()) continue;
        int y = 0, m = 0, d = 0;
        if (std::sscanf(hd->layNgayLap().c_str(), "%d-%d-%d", &y, &m, &d) != 3) continue;
        if (m == thang && y == nam) tong += hd->tinhTongTien();
    }
    return tong;
}

double QuanLyKhachSan::thongKeCongSuat() const {
    int tong = 0, dangO = 0;
    for (const auto& cn : danhSachChiNhanh)
        for (const auto& p : cn->layDanhSachPhong()) {
            ++tong;
            if (p->layTenTrangThai() == TenTrangThai::DANG_O) ++dangO;
        }
    return tong == 0 ? 0.0 : static_cast<double>(dangO) / tong * 100.0;
}

// ------------------------------------------------------------
//  Đọc / ghi file
//  Định dạng văn bản:
//    KHACHSAN_V1
//    CHINHANH <n>  { ma \n ten \n diaChi \n  PHONG <k> {loai + <<Phong}  NHANVIEN <j> {<<NhanVien} }
//    KHACHHANG <n> { <<KhachHang }
//    PHIEU <n>     { maPhieu maKH maPhong checkIn checkOut ngayDat soNguoi tienCoc mua daHuy }
//  Hóa đơn không lưu vì có thể lập lại từ phiếu đặt phòng.
//  Yêu cầu: operator<< / operator>> của Phong, NhanVien, KhachHang ghi/đọc đối xứng nhau.
// ------------------------------------------------------------
void QuanLyKhachSan::luuFile(const std::string& tenFile) const {
    std::ofstream out(tenFile);
    if (!out) throw std::runtime_error("Khong mo duoc file de ghi: " + tenFile);

    out << "KHACHSAN_V1\n";

    out << "CHINHANH " << danhSachChiNhanh.size() << "\n";
    for (const auto& cn : danhSachChiNhanh) {
        out << cn->layMaChiNhanh() << "\n" << cn->layTenChiNhanh() << "\n" << cn->layDiaChi() << "\n";
        out << "PHONG " << cn->layDanhSachPhong().size() << "\n";
        for (const auto& p : cn->layDanhSachPhong())
            out << static_cast<int>(p->layLoaiPhong()) << "\n" << *p << "\n";
        out << "NHANVIEN " << cn->layDanhSachNhanVien().size() << "\n";
        for (const auto& nv : cn->layDanhSachNhanVien())
            out << *nv << "\n";
    }

    out << "KHACHHANG " << danhSachKhachHang.size() << "\n";
    for (const auto& kh : danhSachKhachHang)
        out << *kh << "\n";

    out << "PHIEU " << danhSachPhieuDat.size() << "\n";
    for (const auto& ph : danhSachPhieuDat) {
        out << ph->layMaPhieu() << " " << ph->layKhachHang()->layMaSo() << " "
            << ph->layPhong()->layMaPhong() << " " << ph->layNgayCheckIn() << " "
            << ph->layNgayCheckOut() << " " << ph->layNgayDat() << " "
            << ph->laySoNguoi() << " " << ph->layTienCoc() << " "
            << static_cast<int>(ph->layMua()) << " " << ph->kiemTraDaHuy() << "\n";
    }
}

void QuanLyKhachSan::docFile(const std::string& tenFile) {
    std::ifstream in(tenFile);
    if (!in) throw std::runtime_error("Khong mo duoc file de doc: " + tenFile);

    std::string tag;
    std::getline(in, tag);
    if (tag != "KHACHSAN_V1") throw std::runtime_error("File khong dung dinh dang");

    // Xóa dữ liệu cũ
    danhSachChiNhanh.clear(); danhSachKhachHang.clear();
    danhSachPhieuDat.clear(); danhSachHoaDon.clear();
    dichVuTheoPhong.clear();
    demPhieu = demHoaDon = 0;

    size_t n = 0;

    // --- Chi nhánh ---
    in >> tag >> n >> std::ws;
    for (size_t i = 0; i < n; ++i) {
        std::string ma, ten, dc;
        std::getline(in, ma); std::getline(in, ten); std::getline(in, dc);
        auto cn = std::make_shared<ChiNhanhResort>(ma, ten, dc);

        size_t soPhong = 0;
        in >> tag >> soPhong >> std::ws;
        for (size_t j = 0; j < soPhong; ++j) {
            int loai = 0;
            in >> loai >> std::ws;
            std::shared_ptr<Phong> p;
            switch (static_cast<LoaiPhong>(loai)) {
                case LoaiPhong::STANDARD: p = std::make_shared<StandardRoom>(); break;
                case LoaiPhong::DELUXE:   p = std::make_shared<DeluxeRoom>();   break;
                case LoaiPhong::SUITE:    p = std::make_shared<Suite>();         break;
                case LoaiPhong::VILLA:    p = std::make_shared<Villa>();         break;
                default: throw std::runtime_error("Loai phong khong hop le trong file");
            }
            in >> *p >> std::ws;
            cn->themPhong(p);
        }

        size_t soNV = 0;
        in >> tag >> soNV >> std::ws;
        for (size_t j = 0; j < soNV; ++j) {
            auto nv = std::make_shared<NhanVien>();
            in >> *nv >> std::ws;
            cn->themNhanVien(nv);
        }
        danhSachChiNhanh.push_back(cn);
    }

    // --- Khách hàng ---
    in >> tag >> n >> std::ws;
    for (size_t i = 0; i < n; ++i) {
        auto kh = std::make_shared<KhachHang>();
        in >> *kh >> std::ws;
        danhSachKhachHang.push_back(kh);
    }

    // --- Phiếu đặt phòng ---
    in >> tag >> n >> std::ws;
    for (size_t i = 0; i < n; ++i) {
        std::string maPhieu, maKH, maPhong, ci, co, ngayDat;
        int soNguoi, mua, huy;
        double coc;
        in >> maPhieu >> maKH >> maPhong >> ci >> co >> ngayDat >> soNguoi >> coc >> mua >> huy;

        auto kh = timKhachHang(maKH);
        auto p  = timPhongToanHeThong(maPhong);
        if (!kh || !p) continue;   // dữ liệu tham chiếu hỏng -> bỏ qua phiếu

        auto phieu = std::make_shared<PhieuDatPhong>(
            maPhieu, kh, p, nullptr, ngayDat, ci, co, soNguoi, coc, static_cast<MuaDuLich>(mua));
        if (huy) phieu->huyPhieu();
        danhSachPhieuDat.push_back(phieu);
        kh->themLichSu(phieu);

        // Cập nhật bộ đếm để mã sinh mới không bị trùng
        int so = 0;
        if (std::sscanf(maPhieu.c_str(), "PD%d", &so) == 1) demPhieu = std::max(demPhieu, so);
    }
}
