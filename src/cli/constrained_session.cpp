#include "cli/constrained_session.h"

#include "hardware/storage_type.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// MinGW headers may predate Windows 8 job CPU-rate APIs; supply compatible declarations.
#if defined(__MINGW32__) && !defined(JOB_OBJECT_CPU_RATE_CONTROL_ENABLE)
#define JOB_OBJECT_CPU_RATE_CONTROL_ENABLE 0x1u
#define JOB_OBJECT_CPU_RATE_CONTROL_HARD_CAP 0x4u
typedef struct _HAPP_JOBOBJECT_CPU_RATE_CONTROL_INFORMATION {
    DWORD CpuRate;
    DWORD ControlFlags;
} JOBOBJECT_CPU_RATE_CONTROL_INFORMATION;
#ifndef HAPP_JOB_OBJECT_CPU_RATE_CONTROL_CLASS
#define HAPP_JOB_OBJECT_CPU_RATE_CONTROL_CLASS static_cast<JOBOBJECTINFOCLASS>(13)
#endif
#else
#ifndef HAPP_JOB_OBJECT_CPU_RATE_CONTROL_CLASS
#define HAPP_JOB_OBJECT_CPU_RATE_CONTROL_CLASS JobObjectCpuRateControlInformation
#endif
#endif

#endif

bool constrained_session_available() {
#ifdef _WIN32
    return true;
#else
    return false;
#endif
}

std::string constrained_session_limitations_note() {
    return "OS-enforced in this mode: total committed RAM for all processes in the session, "
           "and a CPU usage cap derived from --cpu vs this machine's logical processors. "
           "--storage and --network are not applied by the OS (they still affect the analytic "
           "simulator only).";
}

#ifdef _WIN32

namespace {

std::wstring widen_utf8(const std::string& s) {
    if (s.empty()) {
        return std::wstring();
    }
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), nullptr, 0);
    if (n <= 0) {
        return std::wstring();
    }
    std::wstring w(static_cast<std::size_t>(n), 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), &w[0], n);
    return w;
}

void set_session_env_vars(const HardwareConfig& hw) {
    SetEnvironmentVariableW(L"HAPP_SESSION", L"1");
    SetEnvironmentVariableW(L"HAPP_CPU_CORES", widen_utf8(std::to_string(hw.cpu_cores)).c_str());
    SetEnvironmentVariableW(L"HAPP_RAM_GB", widen_utf8(std::to_string(hw.ram_gb)).c_str());
    SetEnvironmentVariableW(L"HAPP_STORAGE", widen_utf8(storage_type_label(hw.storage)).c_str());
    SetEnvironmentVariableW(L"HAPP_NETWORK_MBPS", widen_utf8(std::to_string(hw.network_mbps)).c_str());
    if (!hw.label.empty()) {
        SetEnvironmentVariableW(L"HAPP_LABEL", widen_utf8(hw.label).c_str());
    }
}

std::string last_error_message(const char* context) {
    const DWORD err = GetLastError();
    if (err == 0) {
        return std::string(context) + ": unknown error";
    }
    LPWSTR buf = nullptr;
    const DWORD fl = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS;
    const DWORD n = FormatMessageW(fl, nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                   reinterpret_cast<LPWSTR>(&buf), 0, nullptr);
    std::string msg = context;
    msg += " (";
    msg += std::to_string(static_cast<unsigned long long>(err));
    msg += "): ";
    if (n && buf) {
        std::wstring w(buf);
        LocalFree(buf);
        int bytes = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), static_cast<int>(w.size()), nullptr,
                                        0, nullptr, nullptr);
        if (bytes > 0) {
            std::string narrow(static_cast<std::size_t>(bytes), '\0');
            WideCharToMultiByte(CP_UTF8, 0, w.c_str(), static_cast<int>(w.size()), &narrow[0],
                                bytes, nullptr, nullptr);
            while (!narrow.empty() &&
                   (narrow.back() == '\n' || narrow.back() == '\r' || narrow.back() == '\0')) {
                narrow.pop_back();
            }
            msg += narrow;
            return msg;
        }
    }
    msg += "Windows API error";
    return msg;
}

}  // namespace

int run_constrained_interactive_session(const HardwareConfig& hw, std::string& error) {
    set_session_env_vars(hw);

    HANDLE job = CreateJobObjectW(nullptr, nullptr);
    if (!job) {
        error = last_error_message("CreateJobObjectW failed");
        return -1;
    }

    const double ram_gb = std::max(0.25, hw.ram_gb);
    const std::uint64_t ram_bytes =
        static_cast<std::uint64_t>(std::llround(ram_gb * 1024.0 * 1024.0 * 1024.0));

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli{};
    jeli.BasicLimitInformation.LimitFlags =
        JOB_OBJECT_LIMIT_JOB_MEMORY | JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    jeli.JobMemoryLimit = static_cast<SIZE_T>(std::min<std::uint64_t>(
        ram_bytes, static_cast<std::uint64_t>(SIZE_MAX)));

    if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli))) {
        error = last_error_message("SetInformationJobObject (memory) failed");
        CloseHandle(job);
        return -1;
    }

    SYSTEM_INFO sys{};
    GetSystemInfo(&sys);
    const int logical = std::max(1, static_cast<int>(sys.dwNumberOfProcessors));
    const int want_cores = std::max(1, hw.cpu_cores);
    const double cap_frac =
        logical > 0 ? std::min(1.0, static_cast<double>(want_cores) / static_cast<double>(logical))
                    : 1.0;
    const DWORD cpu_rate = static_cast<DWORD>(std::llround(cap_frac * 10000.0));
    JOBOBJECT_CPU_RATE_CONTROL_INFORMATION cpu{};
    cpu.ControlFlags = JOB_OBJECT_CPU_RATE_CONTROL_ENABLE | JOB_OBJECT_CPU_RATE_CONTROL_HARD_CAP;
    cpu.CpuRate = std::max<DWORD>(1, std::min<DWORD>(10000, cpu_rate));

    if (!SetInformationJobObject(job, HAPP_JOB_OBJECT_CPU_RATE_CONTROL_CLASS, &cpu, sizeof(cpu))) {
        // CPU rate control is unavailable on some SKUs; continue with memory limits only.
        // (Do not fail the whole session.)
    }

    wchar_t comspec[MAX_PATH + 4] = {};
    DWORD comspec_len = GetEnvironmentVariableW(L"COMSPEC", comspec, MAX_PATH);
    if (comspec_len == 0 || comspec_len >= MAX_PATH) {
        const wchar_t* fallback = L"C:\\Windows\\System32\\cmd.exe";
        wcsncpy(comspec, fallback, MAX_PATH);
        comspec[MAX_PATH] = L'\0';
    }

    std::wstring app = comspec;
    std::wstring cmd = L"\"";
    cmd += app;
    cmd += L"\" /K";

    std::vector<wchar_t> cmd_mut(cmd.begin(), cmd.end());
    cmd_mut.push_back(L'\0');

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};

    // If `happ` is already in a job, child must break away before we can assign our own job.
    // If not in a job, CREATE_BREAKAWAY_FROM_JOB is documented as having no effect (MinGW may
    // lack IsProcessInJob in the import library, so we always set the flag).
    const DWORD create_flags = CREATE_SUSPENDED | CREATE_BREAKAWAY_FROM_JOB;

    if (!CreateProcessW(app.c_str(), &cmd_mut[0], nullptr, nullptr, TRUE, create_flags, nullptr,
                         nullptr, &si, &pi)) {
        error = last_error_message("CreateProcessW failed");
        CloseHandle(job);
        return -1;
    }

    if (!AssignProcessToJobObject(job, pi.hProcess)) {
        error = last_error_message(
            "AssignProcessToJobObject failed (is another job wrapping this terminal?)");
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        CloseHandle(job);
        return -1;
    }

    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code = 1;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hProcess);
    CloseHandle(job);

    return static_cast<int>(exit_code);
}

#else  // !_WIN32

int run_constrained_interactive_session(const HardwareConfig&, std::string& error) {
    error =
        "Interactive constrained sessions are only implemented on Windows (Job Objects). "
        "On this platform, use containers/cgroups or a VM, and keep using `happ simulate` "
        "for analytic estimates.";
    return -1;
}

#endif
