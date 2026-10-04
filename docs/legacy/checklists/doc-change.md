# Documentation change checklist

- [ ] Fact được đối chiếu với source/config/schema/test đang build.
- [ ] Trạng thái Source/Deployed/HIL được cập nhật độc lập.
- [ ] Không suy diễn HIL từ build hoặc service active.
- [ ] Không chứa secret, dataset, token, password, private key hoặc grant.
- [ ] Lệnh thay đổi trạng thái nằm trong runbook có rollback.
- [ ] Diagram canonical được cập nhật hoặc link, không sao chép.
- [ ] Link relative tồn tại.
- [ ] Toolchain pin đọc từ script/lockfile.
- [ ] Tài liệu legacy có banner và link thay thế.
- [ ] `python3 tools/check_docs.py` pass.
- [ ] Không tuyên bố production-ready/compliance nếu thiếu evidence.