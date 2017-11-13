////////////////////////////////////////////////////////////////////////////////
//
//  kadai.cpp
//	todo: おえかき部分の背景色の設定をどうするか
//			おえかき部分を起動時などに無効化する方法
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
//  ヘッダファイル
//
#include <Windows.h>
#include <WinSock.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////
//
// 使用ライブラリ
//
#pragma comment(lib,"wsock32.lib")

////////////////////////////////////////////////////////////////////////////////
//
//  定数定義
//
#define WM_SOCKET       (WM_USER+1)     // ソケット用メッセージ
#define PORT            10000           // 通信ポート番号

#define IDB_CONNECT     1000            // [接続]ボタン
#define IDB_ACCEPT      1001            // [接続待ち]ボタン
#define IDB_REJECT      1002            // [切断]ボタン
#define IDB_SEND        1003            // [送信]ボタン
#define IDB_REJECT_REQUEST 1004			// [切断要請]ボタン

#define IDF_HOSTNAME    2000                // ホスト名入力エディットボックス
#define IDF_SENDMSG     2001            // 送信メッセージ入力用エディットボックス
#define IDF_RECVMSG     2002            // 受信メッセージ表示用エディットボックス

#define IDE_RECVMSG     3000            // メッセージ受信イベント

#define WINDOW_W        400         // ウィンドウの幅
#define WINDOW_H        1000         // ウィンドウの高さ

#define MAX_MESSAGE     128         // テキストメッセージの配列の最大要素数
#define MAX_POS			10000		// 座標の最大数

////////////////////////////////////////////////////////////////////////////////
//点のデータを保存するクラス
//
class Data {
private:
	int flag[MAX_POS];		//線の始点かどうかのフラグ
	POINT pos[MAX_POS];		//キャンバス上の点の座標
	int n;					//点の個数

public:
	//データをセットするメソッド
	void setData(int f, int x, int y) {
		flag[n] = f;
		pos[n].x = x;
		pos[n].y = y;
	}

	//フラグを取得するメソッド
	int getFlag(int i) {
		return flag[i];
	}

	//点の座標を取得するメソッド
	POINT getPos(int i) {
		return pos[i];
	}

	//点の個数を取得するメソッド
	int getNumberOfPoint() {
		return n;
	}

	//点の個数を増やすメソッド
	void inclimentNum();
};

void Data::inclimentNum() {
	n++;
}

////////////////////////////////////////////////////////////////////////////////
//
//  グローバル変数
//
LPCTSTR lpClassName = "OekakiChat";        // ウィンドウクラス名
LPCTSTR lpWindowName = "OekakiChat";        // タイトルバーにつく名前

SOCKET sock = INVALID_SOCKET;            // ソケット
SOCKET sv_sock = INVALID_SOCKET;            // サーバ用ソケット
HOSTENT *phe;                       // HOSTENT構造体

HPEN hPenBlack;
HPEN hPenRed;

const RECT d = { 10, 300, 355, 550 };	//キャンバスの範囲

Data myData;		//自分が描いた点
Data recvData;		//相手が描いた点

////////////////////////////////////////////////////////////////////////////////
//
//  プロトタイプ宣言
//
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);   // ウィンドウ関数
BOOL SockInit(HWND hWnd);                               // ソケット初期化
BOOL SockAccept(HWND hWnd);                             // ソケット接続待ち
BOOL SockConnect(HWND hWnd, LPCSTR host);               // ソケット接続

LRESULT CALLBACK OnPaint(HWND, UINT, WPARAM, LPARAM);
//void setData(int flag, int x, int y);
//void setData_p(int flag, int x, int y);
BOOL checkMousePos(int x, int y);

void WindowInit(HWND hWnd);                             // ウィンドウ初期化


////////////////////////////////////////////////////////////////////////////////
//
//  WinMain関数 (Windowsプログラム起動時に呼ばれる関数)
//
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd;                                          // ウィンドウハンドル
	MSG  msg;                                           // メッセージ
	WNDCLASSEX wc;                                      // ウィンドウクラス

														//ウィンドウクラス定義
	wc.hInstance = hInstance;                       // インスタンス
	wc.lpszClassName = lpClassName;                     // クラス名
	wc.lpfnWndProc = WindowProc;                      // ウィンドウ関数名
	wc.style = 0;                               // クラススタイル
	wc.cbSize = sizeof(WNDCLASSEX);              // 構造体サイズ
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); // アイコンハンドル
	wc.hIconSm = LoadIcon(NULL, IDI_WINLOGO);     // スモールアイコン
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);      // マウスポインタ
	wc.lpszMenuName = NULL;                            // メニュー(なし)
	wc.cbClsExtra = 0;                               // クラス拡張情報
	wc.cbWndExtra = 0;                               // ウィンドウ拡張情報
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;           // ウィンドウの背景色
	if (!RegisterClassEx(&wc)) return 0;                // ウィンドウクラス登録

														// ウィンドウ生成
	hWnd = CreateWindow(
		lpClassName,                                // ウィンドウクラス名
		lpWindowName,                               // ウィンドウ名
		WS_DLGFRAME | WS_VISIBLE | WS_SYSMENU,          // ウィンドウ属性
		CW_USEDEFAULT,                              // ウィンドウ表示位置(X)
		CW_USEDEFAULT,                              // ウィンドウ表示位置(Y)
		WINDOW_W,                                   // ウィンドウサイズ(X)
		WINDOW_H,                                   // ウィンドウサイズ(Y)
		HWND_DESKTOP,                               // 親ウィンドウハンドル
		NULL,
		hInstance,                                  // インスタンスハンドル
		NULL
		);

	// ウィンドウ表示
	ShowWindow(hWnd, nCmdShow);                         // ウィンドウ表示モード
	UpdateWindow(hWnd);                                 // ウインドウ更新

														// メッセージループ
	while (GetMessage(&msg, NULL, 0, 0)) {                // メッセージを取得
		TranslateMessage(&msg);
		DispatchMessage(&msg);                          // メッセージ送る
	}
	return (int)msg.wParam;                             // プログラム終了
}

////////////////////////////////////////////////////////////////////////////////
//
//  ウィンドウ関数(イベント処理を記述)
//
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wP, LPARAM lP)
{
	static HWND hWndHost;                       // ホスト名入力用エディットボックス
	static HWND hWndConnect, hWndAccept;                // [接続]ボタンと[接続待ち]ボタン
	static HWND hWndReject;                     // [切断]ボタン
	static HWND hWndRejectRequest;				// [切断要請]ボタン
	static HWND hWndSend;                       // [送信]ボタン
	static HWND hWndSendMSG;                    // 送信メッセージ入力用エディットボックス
	static HWND hWndRecvMSG;                    // 受信メッセージ表示用エディットボックス

	static BOOL mouseFlg = FALSE;

	char buf_draw[MAX_MESSAGE];
	switch (uMsg) {
	case WM_CREATE:     // ウィンドウが生成された
						// 文字列表示
		CreateWindow("static", "Host Name",
			WS_CHILD | WS_VISIBLE, 10, 10, 100, 18,
			hWnd, NULL, NULL, NULL);
		CreateWindow(TEXT("static"), TEXT("Send Message"), WS_CHILD | WS_VISIBLE,
			10, 590, 200, 18, hWnd, NULL, NULL, NULL);
		CreateWindow(TEXT("static"), TEXT("Receive Message"), WS_CHILD | WS_VISIBLE,
			10, 780, 200, 18, hWnd, NULL, NULL, NULL);

		// ホスト名入力用エディットボックス
		hWndHost = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "",
			WS_CHILD | WS_VISIBLE, 10, 30, 200, 25,
			hWnd, (HMENU)IDF_HOSTNAME, NULL, NULL);
		// [接続]ボタン
		hWndConnect = CreateWindow("button", "接続",
			WS_CHILD | WS_VISIBLE, 220, 30, 50, 25,
			hWnd, (HMENU)IDB_CONNECT, NULL, NULL);
		// [接続待ち]ボタン
		hWndAccept = CreateWindow("button", "接続待ち",
			WS_CHILD | WS_VISIBLE, 275, 30, 90, 25,
			hWnd, (HMENU)IDB_ACCEPT, NULL, NULL);
		// [送信]ボタン
		hWndSend = CreateWindow("button", "送信",
			WS_CHILD | WS_VISIBLE | WS_DISABLED, 275, 730, 90, 25,
			hWnd, (HMENU)IDB_SEND, NULL, NULL);
		// [切断要請]ボタン
		hWndRejectRequest = CreateWindow("button", "切断要請",
			WS_CHILD | WS_VISIBLE | WS_DISABLED, 10, 930, 90, 25,
			hWnd, (HMENU)IDB_REJECT_REQUEST, NULL, NULL);
		// [切断]ボタン
		hWndReject = CreateWindow("button", "切断",
			WS_CHILD | WS_VISIBLE | WS_DISABLED, 275, 930, 90, 25,
			hWnd, (HMENU)IDB_REJECT, NULL, NULL);
		SetFocus(hWndHost);


		//送信メッセージ入力用エディットボックス
		hWndSendMSG = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT(""),
			WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_DISABLED, 10, 610, 355, 100,
			hWnd, (HMENU)IDF_SENDMSG, NULL, NULL);
		//受信メッセージ表示用エディットボックス
		hWndRecvMSG = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT(""),
			WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY, 10, 800, 355, 100,
			hWnd, (HMENU)IDF_RECVMSG, NULL, NULL);

		SetFocus(hWndHost);     //フォーカス指定
		SockInit(hWnd);         // ソケット初期化

		//ペン生成
		hPenBlack = (HPEN)CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
		hPenRed = (HPEN)CreatePen(PS_SOLID, 3, RGB(255, 0, 0));

		return 0L;

	case WM_LBUTTONDOWN:

		if (checkMousePos(LOWORD(lP), HIWORD(lP))) {
			myData.setData(0, LOWORD(lP), HIWORD(lP));

			int num = myData.getNumberOfPoint();
			sprintf(buf_draw, "%1d%03d%03d\0", myData.getFlag(num), myData.getPos(num).x, myData.getPos(num).y);
			send(sock, buf_draw, strlen(buf_draw) + 1, 0);

			myData.inclimentNum();
			InvalidateRect(hWnd, &d, FALSE);
			mouseFlg = TRUE;
		}
		else {
			mouseFlg = FALSE;
		}

		return 0L;

	case WM_MOUSEMOVE:
		if (wP == MK_LBUTTON) {
			if (checkMousePos(LOWORD(lP), HIWORD(lP))) {
				if (mouseFlg) {
					myData.setData(1, LOWORD(lP), HIWORD(lP));
				}
				else {
					myData.setData(0, LOWORD(lP), HIWORD(lP));
				}
				mouseFlg = TRUE;

				int num = myData.getNumberOfPoint();
				sprintf(buf_draw, "%1d%03d%03d\n", myData.getFlag(num), myData.getPos(num).x, myData.getPos(num).y);
				send(sock, buf_draw, strlen(buf_draw) + 1, 0);

				myData.inclimentNum();
				InvalidateRect(hWnd, &d, FALSE);
			}
			else {
				mouseFlg = FALSE;
			}
		}

		return 0L;

	case WM_COMMAND:    // ボタンが押された
		switch (LOWORD(wP)) {
		case IDB_ACCEPT:    // [接続待ち]ボタン押下(サーバー)
			if (SockAccept(hWnd)) {  // 接続待ち要求
				return 0L;      // 接続待ち失敗
			}
			EnableWindow(hWndHost, FALSE);    // [HostName]無効
			EnableWindow(hWndConnect, FALSE); // [接続]    無効
			EnableWindow(hWndAccept, FALSE);  // [接続待ち]無効
			EnableWindow(hWndReject, TRUE);   // [切断]    有効
			EnableWindow(hWndRejectRequest, TRUE);
			return 0L;

		case IDB_CONNECT:   // [接続]ボタン押下(クライアント)
			char host[100];
			GetWindowText(hWndHost, host, sizeof(host));

			if (SockConnect(hWnd, host)) {   // 接続要求
				SetFocus(hWndHost);     // 接続失敗
				return 0L;
			}
			EnableWindow(hWndHost, FALSE);    // [HostName]無効
			EnableWindow(hWndConnect, FALSE); // [接続]    無効
			EnableWindow(hWndAccept, FALSE);  // [接続待ち]無効
			EnableWindow(hWndReject, TRUE);   // [切断]    有効
			EnableWindow(hWndRejectRequest, TRUE);
			return 0L;

		case IDB_REJECT:    // [切断]ボタン押下
			if (sock != INVALID_SOCKET) {    // 自分がクライアント側なら
											 // ソケットを閉じる
				closesocket(sock);
				sock = INVALID_SOCKET;
			}
			if (sv_sock != INVALID_SOCKET) { // 自分がサーバ側なら
											 // サーバ用ソケットを閉じる
				closesocket(sv_sock);
				sv_sock = INVALID_SOCKET;
			}
			phe = NULL;
			EnableWindow(hWndHost, TRUE);       // [HostName]有効
			EnableWindow(hWndConnect, TRUE);    // [接続]    有効
			EnableWindow(hWndAccept, TRUE);     // [接続待ち]有効
			EnableWindow(hWndReject, FALSE);    // [切断]    無効
			EnableWindow(hWndSendMSG, FALSE);   // 送信文入力不可
			EnableWindow(hWndSend, FALSE);      // [送信]    無効
			SetFocus(hWndHost);         // フォーカス指定
			return 0L;

		case IDB_SEND:      // [送信]ボタン押下
			char buf[MAX_MESSAGE];                  // 送信内容を一時的に格納するバッファ
			GetWindowText(hWndSendMSG, buf, sizeof(buf) - 1);     // 送信メッセージ入力欄の内容を取得
			if (send(sock, buf, strlen(buf) + 1, 0) == SOCKET_ERROR) {    // 送信処理
																		  // 送信に失敗したらエラーを表示
				MessageBox(hWnd, TEXT("sending failed"), TEXT("Error"),
					MB_OK | MB_ICONEXCLAMATION);
			}
			SetWindowText(hWndSendMSG, TEXT(""));    // 送信メッセージ入力用エディットボックスを空にする
			SetFocus(hWndSendMSG);          // フォーカス指定
			return 0L;

		case IDB_REJECT_REQUEST:

			if (send(sock, "REJECT", 7, 0) == SOCKET_ERROR) {

				MessageBox(hWnd, TEXT("request failed"), TEXT("Error"),
					MB_OK | MB_ICONEXCLAMATION);
			}

			return 0L;
		} /* end of switch (LOWORD(wP)) */
		return 0L;

	case WM_SOCKET:          // 非同期処理メッセージ
		if (WSAGETSELECTERROR(lP) != 0) { return 0L; }

		switch (WSAGETSELECTEVENT(lP)) {
		case FD_ACCEPT:     // 接続待ち完了通知
		{
			SOCKADDR_IN cl_sin;
			int len = sizeof(cl_sin);
			sock = accept(sv_sock, (LPSOCKADDR)&cl_sin, &len);

			if (sock == INVALID_SOCKET) {
				MessageBox(hWnd, "Accepting connection failed",
					"Error", MB_OK | MB_ICONEXCLAMATION);
				closesocket(sv_sock);
				sv_sock = INVALID_SOCKET;
				EnableWindow(hWndHost, TRUE);       // [HostName]有効
				EnableWindow(hWndConnect, TRUE);    // [接続]    有効
				EnableWindow(hWndAccept, TRUE);     // [接続待ち]有効
				EnableWindow(hWndReject, FALSE);    // [切断]    無効
				EnableWindow(hWndSendMSG, FALSE);   // 送信文入力不可
				EnableWindow(hWndSend, FALSE);      // [送信]    無効
				SetFocus(hWndHost);         // フォーカス指定
				return 0L;
			}

#ifndef NO_DNS
			// ホスト名取得
			phe = gethostbyaddr((char *)&cl_sin.sin_addr, 4, AF_INET);
			if (phe) { SetWindowText(hWndHost, phe->h_name); }
#endif  NO_DNS

			// 非同期モード (受信＆切断）
			if (WSAAsyncSelect(sock, hWnd, WM_SOCKET, FD_READ | FD_CLOSE)
				== SOCKET_ERROR) {
				// 接続に失敗したら初期状態に戻す
				MessageBox(hWnd, "WSAAsyncSelect() failed",
					"Error", MB_OK | MB_ICONEXCLAMATION);
				EnableWindow(hWndHost, TRUE);       // [HostName]有効
				EnableWindow(hWndConnect, TRUE);    // [接続]    有効
				EnableWindow(hWndAccept, TRUE);     // [接続待ち]有効
				EnableWindow(hWndReject, FALSE);    // [切断]    無効
				EnableWindow(hWndSendMSG, FALSE);   // 送信文入力不可
				EnableWindow(hWndSend, FALSE);      // [送信]    無効
				SetFocus(hWndHost);         // フォーカス指定
				return 0L;
			}
			EnableWindow(hWndSendMSG, TRUE);    // 送信文入力可
			EnableWindow(hWndSend, TRUE);       // [送信]有効
			SetFocus(hWndSendMSG);          // フォーカス指定
			return 0L;
		}/* end of case FD_ACCEPT: */

		case FD_CONNECT:    // 接続完了通知
							// 非同期モード (受信＆切断)
			if (WSAAsyncSelect(sock, hWnd, WM_SOCKET, FD_READ | FD_CLOSE)
				== SOCKET_ERROR) {
				// 接続に失敗したら初期状態に戻す
				MessageBox(hWnd, "WSAAsyncSelect() failed",
					"Error", MB_OK | MB_ICONEXCLAMATION);
				EnableWindow(hWndHost, TRUE);       // [HostName]有効
				EnableWindow(hWndConnect, TRUE);    // [接続]    有効
				EnableWindow(hWndAccept, TRUE);     // [接続待ち]有効
				EnableWindow(hWndReject, FALSE);    // [切断]    無効
				EnableWindow(hWndSendMSG, FALSE);   // 送信文入力不可
				EnableWindow(hWndSend, FALSE);      // [送信]    無効
				SetFocus(hWndHost);         // フォーカス指定
				return 0L;
			}
			EnableWindow(hWndSendMSG, TRUE);    // 送信文入力可
			EnableWindow(hWndSend, TRUE);       // [送信]有効
			SetFocus(hWndSendMSG);          // フォーカス指定

			return 0L;

		case FD_READ:       //メッセージ受信
			char buf[MAX_MESSAGE];                  // 受信内容を一時的に格納するバッファ
			if (recv(sock, buf, sizeof(buf) - 1, 0) != SOCKET_ERROR) { // 受信できたなら
				SetWindowText(hWndRecvMSG, buf);        // 受信メッセージ表示用エディットボックスに受信内容を貼り付け

				if (!strncmp(buf, "REJECT", strlen(buf))) {
					MessageBox(hWnd, "切断が予告されました。\nOKを押すと切断します。", "切断予告", MB_OK | MB_ICONEXCLAMATION);

					if (sock != INVALID_SOCKET) {    // 自分がクライアント側なら
													 // ソケットを閉じる
						closesocket(sock);
						sock = INVALID_SOCKET;
					}
					if (sv_sock != INVALID_SOCKET) { // 自分がサーバ側なら
													 // サーバ用ソケットを閉じる
						closesocket(sv_sock);
						sv_sock = INVALID_SOCKET;
					}
					phe = NULL;
					EnableWindow(hWndHost, TRUE);       // [HostName]有効
					EnableWindow(hWndConnect, TRUE);    // [接続]    有効
					EnableWindow(hWndAccept, TRUE);     // [接続待ち]有効
					EnableWindow(hWndReject, FALSE);    // [切断]    無効
					EnableWindow(hWndSendMSG, FALSE);   // 送信文入力不可
					EnableWindow(hWndSend, FALSE);      // [送信]    無効
					SetFocus(hWndHost);         // フォーカス指定

					return 0L;
				}

				//文字列のパースを行う
				char fstr[2];				//受信したメッセージのうち、フラグを表す部分
				char xstr[4], ystr[4];		//座標を表す部分

											//各部分に分解
				memcpy(&fstr[0], &buf[0], 1);
				fstr[1] = '\0';

				for (int i = 0; i < 3; i++) {
					memcpy(&xstr[i], &buf[i + 1], 1);
				}

				for (int i = 0; i < 3; i++) {
					memcpy(&ystr[i], &buf[i + 4], 1);
				}

				xstr[3] = '\0';	ystr[3] = '\0';

				recvData.setData(atoi(fstr), atoi(xstr), atoi(ystr));
				recvData.inclimentNum();
				InvalidateRect(hWnd, &d, FALSE);
			}
			return 0L;

		case FD_CLOSE:      // 切断された
			MessageBox(hWnd, "切断されました。",
				"Information", MB_OK | MB_ICONINFORMATION);
			SendMessage(hWnd, WM_COMMAND, IDB_REJECT, 0); // 切断処理発行
			return 0L;
		}/* end of switch (WSAGETSELECTEVENT(lP)) */
		return 0L;

	case WM_SETFOCUS:   // ウィンドウにフォーカスが来たら
						// ホスト名入力欄が入力可ならフォーカス
						// 不可なら送信メッセージ入力欄にフォーカス
		SetFocus(IsWindowEnabled(hWndHost) ? hWndHost : hWndSendMSG);
		return 0L;

	case WM_PAINT:		//再描画
		return OnPaint(hWnd, uMsg, wP, lP);

		return 0L;

	case WM_DESTROY:    // ウィンドウが破棄された
		closesocket(sock);
		DeleteObject(hPenBlack);
		DeleteObject(hPenRed);
		PostQuitMessage(0);
		return 0L;


	default:
		return DefWindowProc(hWnd, uMsg, wP, lP);  // 標準メッセージ処理
	}/* end of switch (uMsg) */
}


////////////////////////////////////////////////////////////////////////////////
//
//  ソケット初期化処理
//
BOOL SockInit(HWND hWnd)
{
	WSADATA wsa;
	int ret;
	char ret_buf[80];

	ret = WSAStartup(MAKEWORD(1, 1), &wsa);

	if (ret != 0) {
		wsprintf(ret_buf, "%d is the err", ret);
		MessageBox(hWnd, ret_buf, "Error", MB_OK | MB_ICONSTOP);
		exit(-1);
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
//
//  ソケット接続 (クライアント側)
//
BOOL SockConnect(HWND hWnd, LPCSTR host)
{
	SOCKADDR_IN cl_sin; // SOCKADDR_IN構造体

						// ソケットを開く
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {        // ソケット作成失敗
		MessageBox(hWnd, "Socket() failed", "Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	memset(&cl_sin, 0x00, sizeof(cl_sin)); // 構造体初期化
	cl_sin.sin_family = AF_INET;           // インターネット
	cl_sin.sin_port = htons(PORT);       // ポート番号指定

	phe = gethostbyname(host); // アドレス取得

	if (phe == NULL) {
		MessageBox(hWnd, "gethostbyname() failed.",
			"Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}
	memcpy(&cl_sin.sin_addr, phe->h_addr, phe->h_length);

	// 非同期モード (接続)
	if (WSAAsyncSelect(sock, hWnd, WM_SOCKET, FD_CONNECT) == SOCKET_ERROR) {
		closesocket(sock);
		sock = INVALID_SOCKET;
		MessageBox(hWnd, "WSAAsyncSelect() failed",
			"Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	// 接続処理
	if (connect(sock, (LPSOCKADDR)&cl_sin, sizeof(cl_sin)) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			closesocket(sock);
			sock = INVALID_SOCKET;
			MessageBox(hWnd, "connect() failed", "Error", MB_OK | MB_ICONEXCLAMATION);
			return TRUE;
		}
	}
	return FALSE;
}


//再描画関数
LRESULT CALLBACK OnPaint(HWND hWnd, UINT uMsg, WPARAM wP, LPARAM lP)
{
	HDC hdc;
	PAINTSTRUCT ps;

	hdc = BeginPaint(hWnd, &ps);

	//描画領域初期化
	MoveToEx(hdc, d.left, d.top, NULL);
	LineTo(hdc, d.right, d.top);
	LineTo(hdc, d.right, d.bottom);
	LineTo(hdc, d.left, d.bottom);
	LineTo(hdc, d.left, d.top);

	SelectObject(hdc, GetStockObject(WHITE_BRUSH));
	Rectangle(hdc, d.left, d.top, d.right, d.bottom);

	//自分を描いた分を描画
	SelectObject(hdc, hPenBlack);
	for (int i = 0; i < myData.getNumberOfPoint(); i++) {
		if (myData.getFlag(i) == 0) {
			MoveToEx(hdc, myData.getPos(i).x, myData.getPos(i).y, NULL);
		}
		else {
			LineTo(hdc, myData.getPos(i).x, myData.getPos(i).y);
		}
	}

	//相手が描いた分を描画
	SelectObject(hdc, hPenRed);
	for (int i = 0; i < recvData.getNumberOfPoint(); i++) {
		if (recvData.getFlag(i) == 0) {
			MoveToEx(hdc, recvData.getPos(i).x, recvData.getPos(i).y, NULL);
		}
		else {
			LineTo(hdc, recvData.getPos(i).x, recvData.getPos(i).y);
		}
	}

	EndPaint(hWnd, &ps);

	return 0L;
}

//キャンバス内にマウスが入っているかどうかのチェック
BOOL checkMousePos(int x, int y)
{
	if (x >= d.left && x <= d.right
		&& y >= d.top && y <= d.bottom) {
		return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
//
//  接続待ち (サーバ側)
//
BOOL SockAccept(HWND hWnd)
{
	SOCKADDR_IN sv_sin;         // SOCKADDR_IN構造体

								// サーバ用ソケット
	sv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sv_sock == INVALID_SOCKET) { // ソケット作成失敗
		MessageBox(hWnd, "Socket() failed", "Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	memset(&sv_sin, 0x00, sizeof(sv_sin));      // 構造体初期化
	sv_sin.sin_family = AF_INET;           // インターネット
	sv_sin.sin_port = htons(PORT);       // ポート番号指定
	sv_sin.sin_addr.s_addr = htonl(INADDR_ANY); // アドレス指定

	if (bind(sv_sock, (LPSOCKADDR)&sv_sin, sizeof(sv_sin)) == SOCKET_ERROR) {
		closesocket(sv_sock);
		sv_sock = INVALID_SOCKET;
		MessageBox(hWnd, "bind() failed", "Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	if (listen(sv_sock, 5) == SOCKET_ERROR) {
		// 接続待ち失敗
		closesocket(sv_sock);
		sv_sock = INVALID_SOCKET;
		MessageBox(hWnd, "listen() failed", "Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	// 非同期処理モード (接続待ち)
	if (WSAAsyncSelect(sv_sock, hWnd, WM_SOCKET, FD_ACCEPT) == SOCKET_ERROR) {
		closesocket(sv_sock);
		sv_sock = INVALID_SOCKET;
		MessageBox(hWnd, "WSAAsyncSelect() failed",
			"Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}
	return FALSE;
}
