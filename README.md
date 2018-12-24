# network Door lock

상규랑 같이하는 프로젝트...난 임베디드 쪽~
# 2018-10-26
기본 셋팅 
릴레이 초기화 시간 1바이트
IP자동 구분 1바이트 (0:자동 1:고정)
DNS자동 구분 1바이트 (0:자동 1:고정)
소방신호전압 1바이트
공백 6바이트
아이피 (ip , 서브넷 , 게이트웨이) 12바이트
DNS 8바이트
공백 9바이트
동기화 위한 url 100바이트  (Data_Length + Data)
단말기구분이름 100바이트 (Data_Length + Data) (base64로 인코딩 되어있음, 인코딩된 상태 그대로 사용)
========================================230바이트 (전체)

5분마다 동기화
동기화 데이타 구조

[[S]]데이타 수[]시간[]rf번호/yn>rf번호/yn>rf번호/yn>rf번호/yn>........[[E]]

[[S]] 로 시작을 알리며
전체 데이타 수, 시간, 동기화 데이타 순으로 전송되며
데이타 수, 시간, 동기화 데이타는 [] 로 구분됨
데이타에서 RF번호와 y또는 n의 구분은 / 로 구분되며 
데이타와 데이타 는 > 로 구분됨

시간은 동기화 되는 시간으로 웹에서 현재시간을 보냄
형식은 년월일시분초(14자리)
동기화 주소와 함께 최종 동기화 시간을 함께 전송
예) 동기화 주소?SyncDate=시간

처음 동기화시 SyncDate는 0으로 보내고
데이타받을때 같이온 시간 값을 메모리 특정 영역에 저장하고 있다가
5분뒤 동기화 할때 저장된 시간 정보를 SyncDate로 보냄

# 2018-10-31
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

# 2018-11-05
SPI포트변경
....AppData\Local\Arduino15\packages\stm32duino\hardware\STM32F1\2018.9.24\variants\generic_stm32f103c\board
board.h를 첨부된 파일로 덮어씌우기 할 것...

# 2018-11-06
처음 전체 셋팅시 수신 받을 데이타가 많기때문에 나눠서 받는 방법으로 변경

초기 셋팅시
1.  http://body.ibuild.kr/door_control/sync.php?SyncType=1&SyncDate=0
    으로 전송시 수신받을 데이타 양과 시간 데이타가 리턴
    SyncType를 1로 보낼경우 데이타 양과 시간 정보만 리턴

2.  http://body.ibuild.kr/door_control/sync.php?sNo=0&sCount=3&SyncDate=0
    카드번호와 y , n 에 대한 정보를 리턴
    sNo 는 수신받을 데이타의 시작번호 ( 0 ) 부터 시작
    sCount 는 수신받을 데이타의 양
    sNo=0&sCount=3 로 했을경우 0번째 데이타부터 3개의 데이타 수신
    sNo=5&sCount=10 로 했을경우 5번째 데이타부터 10개의 데이타 수신

    수신받을 데이타라 320개 이고 100개씩 받기로 정했다면
    sNo=0&sCount=100 으로 처리 후
    sNo=100&sCount=100 
    sNo=200&sCount=100
    sNo=300&sCount=20   이런식으로 나눠서 데이타 수신

3. 변수에 SyncDate의 값으로 활용할 시간정보를 1번에서 받은 시간 정보 저장

4. 시간정보는 동기화시 SyncDate의 값으로 활용 ( 동기화 할때마다 SyncDate 값은 다시 갱신 )
    SyncDate 이후 변경된 데이타만 수신할것임

5분마다 동기화시
1.  http://body.ibuild.kr/door_control/sync.php?SyncType=1&SyncDate=SyncDate
     으로 수신받을 데이타 양과 시간정보 리턴
     ( 초기화의 1번에서 SyncDat값만 변경 )

2.  초기화의 2번방법 활용

# 2018-12-19
회로도
RS232 => 5핀 헤더핀으로 (3.3v 와 5V둘다나오도록)
부저 => 4핀 헤더핀으로 ( 1번과 2번 , 3번과 4번 연결 )
LD1 => LD1그대로 두고 EXT_LED 처럼 헤더핀추가
BATT1 => 2핀 헤더소켓으로

PCB사이즈 줄일수 있으면 줄여주세요
가능하면 이더넷위치쪽 먼저 나사 홀 가까이까지 줄이고
반대쪽 줄여주세요...


프로그램
카드 찍으면 문 열리던 말던 무조건 SD카드에 로그 작성하면서 서버로 로그 전송
로그 전송 url은 SET 파일 6줄 기록
시간, 카드번호,  device_name, device_serial 함께 전송
시간변수는 log
카드번호는 rf
device_name 은 dn
device_serial 는 ds

로그 정상적으로 저장되면 서버로 보낸 시간정보와 카드번호 리턴 ( 리던형식 : [logs]시간/RF번호[loge] )
리턴받은 시간정보와 카드번호 확인해서 SD카드의 로그 삭제

SD카드에 썼다가 리턴 후 지우는 이유는 리턴이 거의 실시간이지만 혹시 리턴이 늦게 오는경우도 있지 않을까?
또 다른 이유는 네트워크 이유로 로그 전송 안될때 리턴을 얼마동안이나 기다렸다 SD카드에 써야 할지... 계속 대기를 해야 하니..
그냥 무조건 썼다가 리턴 후 지우는것이 좋을것 같음..

1시간 간격으로 전송 실패한 로그 있는지 확인 후 하나씩 서버로 다시 전송