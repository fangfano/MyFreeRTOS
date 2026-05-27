import shutil
import os
import sys

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
BANK_A_SRC = os.path.join(BASE_DIR, "MyAPP", "BankA", "MyAPP_A.bin")
BANK_B_SRC = os.path.join(BASE_DIR, "MyAPP", "BankB", "MyAPP_B.bin")
TARGET_DIR = os.path.join(BASE_DIR, "Target")


def copy_file(src, dst):
    if not os.path.isfile(src):
        print(f"[SKIP] File not found: {src}")
        return False
    src_size = os.path.getsize(src)
    src_time = os.path.getmtime(src)
    if os.path.isfile(dst):
        dst_size = os.path.getsize(dst)
        dst_time = os.path.getmtime(dst)
        if dst_size == src_size and dst_time == src_time:
            print(f"[SAME] {os.path.basename(dst)} (size={src_size}, already up-to-date)")
            return True
    shutil.copy2(src, dst)
    print(f"[COPY] {src} -> {dst} (size={src_size})")
    return True


def main():
    os.makedirs(TARGET_DIR, exist_ok=True)

    print("=" * 50)
    print("  Brain Servo Firmware Copy Tool")
    print("=" * 50)

    ok = True
    ok &= copy_file(BANK_A_SRC, os.path.join(TARGET_DIR, "MyAPP_A.bin"))
    ok &= copy_file(BANK_B_SRC, os.path.join(TARGET_DIR, "MyAPP_B.bin"))

    if not ok:
        print("\n[WARN] Some files were not found. Please build the project first.")
        sys.exit(1)

    print("\n[DONE] All firmware files copied to Target/")


if __name__ == "__main__":
    main()