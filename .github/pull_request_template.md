## 📝 변경 사항 요약 (Description)
<!-- 이 PR이 해결하는 문제나 추가되는 시스템(예: RHI 추상화, 메모리 할당자 등)을 간략히 설명해주세요. -->
- 

## 🔗 관련 이슈 (Related Issues)
<!-- 관련된 이슈 번호를 적어주세요. (예: Resolves #123) -->
- Resolves #

## 🛠 변경 카테고리 (Type of Change)
<!-- 해당되는 항목에 x를 표시해주세요. [x] -->
- [ ] 🐛 버그 수정 (Bug Fix)
- [ ] ✨ 새로운 기능 (New Feature)
- [ ] ♻️ 리팩토링 (Refactoring - 아키텍처 개선, 내부 구조 변경 등)
- [ ] ⚡ 성능 개선 (Performance Improvement)
- [ ] 🛠 빌드/환경 설정 변경 (CMake 타겟 설정, 컴파일 옵션 등)
- [ ] 📝 문서 업데이트 (Documentation)

## 🖥 플랫폼 호환성 (Platform Compatibility)
<!-- 크로스 플랫폼 지원을 위해 테스트를 진행한 환경에 체크해주세요. -->
- [ ] Windows
- [ ] macOS
- [ ] Linux
- [ ] 기타 (명시: )

## ⚙️ 메모리 및 성능 영향 (Memory & Performance Impact)
<!-- 코어 시스템에 미치는 영향을 명시해주세요. -->
- **메모리:** <!-- 예: 커스텀 메모리 할당자(FMalloc) 정상 동작 확인, 누수 테스트 완료 등 -->
- **성능/아키텍처:** <!-- 예: RHI 레이어 오버헤드 측정 결과, 리플렉션 시스템 변경 사항 등 -->

## ✅ 리뷰 체크리스트 (Review Checklist)
- [ ] 헤더 포함(Include) 규칙 및 전방 선언(Forward Declaration)을 올바르게 사용했는가?
- [ ] 헤더 파일 내의 불필요한 인라인 구현을 피하고, 로직을 `.cpp` 파일로 적절히 분리했는가?
- [ ] CMake `target_compile_definitions` 및 `include` 경로 설정 등이 깔끔하게 유지되고 있는가?
- [ ] 모든 플랫폼에서 컴파일 경고(Warning) 없이 빌드가 통과하는가?

## 📸 스크린샷 또는 추가 정보 (Screenshots / Additional Notes)
<!-- 렌더링 결과물이나, 리뷰어가 특별히 주의 깊게 봐야 할 아키텍처 설계 의도가 있다면 남겨주세요. -->

## ✅ 기능 단위 테스트 항목(Unit Test)
 -
 
 ## 🛠 해당 기능 적용 예정 폴더
 - 전체 폴더