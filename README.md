# nano_enc28j60
상규랑 같이하는 프로젝트...난 임베디드 쪽~

2018-10-31
arduino nano v3에서 stm32f103 bluepill 보드로 변경후 중간결과
1. bulepill(stm32duino)는 2가지 버젼의 라이브러리가 존재함
   - maple lib : https://github.com/rogerclarkmelbourne/Arduino_STM32
    (한글자료 https://m.blog.naver.com/elecraft/220965121302 참고)
   - stm lib(?? github에 있는데 링크를 모르겠음)
    (자료 https://wiki.stm32duino.com/index.php?title=Blue_Pill#Hardware_installation 참고)
  
   일단 maple라이브러리를 이용하여 부트로더 올리고 usb시리얼로 프로그램 업로드하는 것으로 진행
    STM라이브러리가 더 좋아보이기는 하는데 stlink를 써야해서 하고
    아두이노 코드를 그대로 활용하기 위함도 있음
  
2. freertos900사용 -> 메모리 부족시 하드코딩을 변경예정
3. PN532를 serial3을 이용하여 동작확인 -> I2C와 결선은 동일 코드만 수정됨
4. SDcard를 spi2를 이용하여 동작확인 -> SD 라이브러리 수정이 있었음
5. os event를 사용하여 uid 인식시 sdcard를 읽도록 수정후 동작확인