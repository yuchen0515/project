# 迫著空間加速MCTS算法之5五將棋應用
* Author: Yu-Chen Lin 林育辰
* Instructor: Shun-Shii Lin 林順喜教授

## 投影片
* [Available](https://docs.google.com/presentation/d/16CuZX9PbxWSvQEs8notf7pUVr0hBSYzq?rtpof=true&authuser=40771131h%40gapps.ntnu.edu.tw&usp=drive_fs)

## Demo

![Demo-video](https://github.com/yuchen0515/project/blob/interface/project-demo.gif?raw=true)

* 本專題**尚未完成**，目前僅完成 SDL 之介面操作
* 介面操作包含將棋之升變、俘虜軍、走步提示和遊戲結束

## 名詞解釋

* 迫著空間：棋局中較為關鍵的走步
* MCTS：全名為 蒙地卡羅樹搜尋法 (Monte Carlo tree search)，是目前廣泛運用於電腦對局、基於CNN(卷積神經網路)圖像辨識的搜索法
* 5五將棋：其原身為日本將棋，為 9*9 之棋盤，而後延伸出 5*5 棋盤，共有王、金、銀、角、飛和步六種兵種的對局遊戲



---

## 書面報告

<center><font size=6><b>迫著空間加速MCTs算法之 5 五將棋應用</b></font></center>

<br>

<center><font size=3><b>林育辰</b> <i>Yu-Chen Lin</i></font></center>

<br>

<center><font size=><b>指導教授：林順喜</b> <i>Shun-Shii Lin</i></font></center>

##### 壹、摘要

​		5 五將棋 (Mini-Shogi) 其原身為日本將棋，起源於寶應象棋或印度恰圖蘭卡，而後傳入日本發展而來，其最為人稱道的規則便是「俘虜軍」、「升變」等特殊規則，可藉由吃子將對方棋子納為己用，因此棋盤上不會消失任何一子，意即比賽過程中必有一方有足夠棋力能夠將死對方，對弈很少發生和局，也因錯綜繁雜的規則，使每一次的搜索分支數量都很大，在常見的棋類中，博弈樹複雜度 226 僅次於圍棋的 360。

​		AlphaGo 在 2016、2017年分別擊敗世界棋王李世乭、柯潔，從此 Machine Learning的前景備受看好，挹注了人們的光彩。近期由 DeepMind 提出了 MuZero算法，是基於 AlphaZero的迭代算法、整合學習好的模型，並能在不給出遊戲規則的情況下，對弈各種棋類，「事前已知遊戲規則」的 AlphaZero棋力已無法與其同日而喻。人工智慧領域在 Deep Blue (IBM開發) 於 1996年擊敗了西洋棋世界冠軍 Garry Kasparov，也引燃了探索這塊領域的導火線。

​		而 5 五將棋 (Mini-Shogi) 則是在1970年左右發展出的分支，並在 TAAI (Technology ans Applications of Artificial Intelligence) 和 TCGA (Taiwan Computer Games Association) 等對局競賽項目列為正式比賽項目之一，近期相關研究則包括臺師大研究生張懷文所研製的 Wonders，是基於Alpha-beta Pruning所開發，而仍有尚需改良之處，包括搜索程式與 Deep Learning的校接，開發開局庫、殘局庫以厚實賽事棋力之基底，目前 5 五將棋較欠缺圖形化介面、可線上對弈的相關資源。

​		冀望在林順喜教授指導下，吸收諸位先進的優勢，並改量不足之處，本專題將著重在加速MCTs算法的研究上。



**關鍵詞**：電腦對局、5 五將棋、蒙地卡羅樹搜尋、機器學習



---

##### 貳、研究動機與研究問題

​		對局競賽中，許多對弈項目所限制決策秒數甚低，以致於「背景運算」無較大改進空間，一般以列表、窮舉等方式以盼降低其運算時間，而若欲以「背景運算」擴展己方之運算時間，則若其他玩家決策時間短，實則無增加多少可運用時間。

​		而 5 五將棋則有較多的思考時間，因此可利用「背景思考」多加運用對方的決策時間同時運算，然對局隨著棋局的多變，在某些情況下可能會耗費極多的決策時間，以致超時，本研究亦因應而生，冀望能協助搜索算法提高其運算效率，以解決問題。

​		在搜索程式中，有兩項主流的算法，一為 *MCTs* 即蒙地卡羅樹搜索法，其中所尋找的是「搜索到目前為止的近似解」，二則為 *Alpha-beta pruning* 專攻於尋找「最佳解」，在這類算法中也產生出許多變形，更可輕易與 *Deep Learning* 相互接應，其影響力不容小覷，而本文則看中 *MCTs* 的優勢，能返回「當前搜索近似最佳解的結果」，此類方式應較適合應用於 5 五將棋上。

<center><font size=2>1</font></center>
<div style="page-break-after:always;"></div>

---

##### 参、文獻回顧與探討

###### 3.1 蒙地卡羅樹搜尋法 (Monte Carlo Tree Search)

​		*MCTs (Monte Carlo Tree Search)* 為一種搜索「近似」最佳解的算法，其算法實現主要涵括了「選擇」、「擴展」、「模擬」和「反向傳遞」去實現。

1. Seclection：從根節點遞迴選擇當下最優子節點，直到抵達葉子節點。
2. Expansion：若葉子節點並非「終止」節點則創建或擴展更多子節點，並選擇其中一個。
3. Simulation：從上述的選擇開始模擬輸出，直到遊戲結束。
4. Backpropagation：將模擬結果輸出，並更新當前序列。



​		而在上述步驟中，遍歷節點倚靠的是「選擇最大化某個量」來實現「可能的最佳選擇」，因此過程中必須「最大化」每一輪的預期收益，一般以 *UCB (Upper Confidence Bounds* 為主要方式：

![](https://i.imgur.com/zFcngxJ.png)


+ $u_i$：節點預估值
+ $n_i$：節點拜訪次數
+ $N$：父節點已被拜訪總次數
+ $C$：參數



​        當 $C$ 越高，則傾向探索原有的節點，反之則傾向探索「新」節點，我們可以透過調整 *UCB* 的參數，以期 *MCTs* 算法能盡量聚焦於我們關注的節點上，進一步開拓更有潛力之節點以達到勝利。



###### 3.2 Alpha-Beta Pruning

​		奠基於 Minimax 搜索法的改良版本，於此演算法中，其最大目的在於使己方收益盡可能最大化，而對方則盡可能最小，藉此開拓對己方最有益之局面，而在 Alpha-Beta Pruning 搜索法中，則新增 $\alpha$ 及 $\beta$ 兩個參數作為剪枝之參考基準，便能在「確保剪枝不影響到最終結果」的情況下，適當減少運算量，除能確保找出好的局面，亦能保障其搜索效率。













<center><font size=2>2</font></center>
<div style="page-break-after:always;"></div>

###### 3.3 Bitboard

​		Bitboard 是透過 $1$ 和 $0$ 兩種狀態，貯存盤面之狀況，並以遮罩加上位元運算，藉此能快速產生走步、棋盤資訊，故受廣為使用。而於本研究中，5 五將棋共有十二隻棋子，針對每個棋子，共有 $25$ bits 去儲存此棋於棋盤上位置之狀況，其餘 $7$ bits，用以儲存升變、陣營等資訊，而在 [1] 中，提及使用 *Bitboard* 時，常需在棋盤編號以及 *unsigned integer* 作轉換，直接將 *Bitboard* 轉換的整數並不是我們所需要的值，在該研究實作中，運用了 *perfect hash* 使 $\text{0~31}$ 能夠對應到相對的位置。

![](https://i.imgur.com/pb62myw.png)




###### 3.4 打入

​		[1] 的文獻中，探討了如何針對「打入」合理的控制搜索複雜度，而「控制打入的搜索複雜度」問題之所以重要，在於 5 五將棋 的「打入」能使吃進的棋子，重新返回我方棋盤上，因此在遊戲過程中，並不會因為時間推進而降低場上存在的棋子數量，根據文獻探討之內容，一個打入可能會多出 $20$ 個左右的分支，即便每層只增加一顆打入，其分支仍會以 $(20+x)^n$ 的指數成長，當手頭握有許多俘虜軍未打入，則對於程式本身，會增加許多運算的成本。

​		該文獻處理方式是對於「搜索給予層數」作限制，如只有前三層會有打入步，而其後之打入狀況則不考慮，其原因在於若能連續兩子打入使對方的王將處於劣勢的話，途中對方亦可以打入的方式破除局面，但我們期望能在進行這「三手」後形成「對敵方王將產生高壓力」的局面，這是在搜索過程中期望優先搜索到的。

![](https://i.imgur.com/U3OrhA6.png)







<center><font size=2>3</font></center>
<div style="page-break-after:always;"></div>

​		本研究除上述研究動機外，另一主要原因也希望能改良過去以 *Alpha-beta pruning* 搜索法實作的 5 五將棋，並將各項功能改良並加上文獻中提及尚未完成之部分，在該環節中，主要探討之論文對象為 Design and Implementation of Computer Minishogi “Wonders”, 2014, 張懷文撰、林順喜教授指導。



###### 3.5 Transposition Table

​		此外[1]在文獻實作中，解決了對局程式較難處理的問題，即在遊戲搜索樹中，可能會以不同的走步順序，但產生出相同的盤面，其運用了 *Zobrish Hash Key* 使用 *unsigned integer*，並依據遊戲類型給予數量不等之變數，每個棋在不同的位置都會有不同的亂數，經由所有棋子的亂數 *XOR* 運算以後，得到的便是棋盤狀況，而當遇到 *collision* 即衝突之狀況時，則會將原本的 $32$ bits 亂數，擴展為 $64$ bits，而前 $32$ bits為檢查碼，後 $32$ bits為資料，在該文應用於 *Wonder* 程式上的實作結果來看，結果相當成功，能將 $11$ 層的搜索層數擴展到 $13$ 層。



###### 3.6 依盤面局勢重要性之採樣的訓練策略

​		於[2]文獻中提到一個在 5 五將棋 十分重要的問題，由於 5 五將棋相當仰賴中盤的下棋決策，稍有閃失在較強的AI面前就會敗北，因此本文作者將中盤的樣本資料重要程度調高些，並針對每局回合數進行開局、中盤、殘局的劃分，切割完後每個回合數都會因切分位置不同而有不同的機率，而依照這些權重重新抽樣後，就會將原樣本變成新的樣本，最終給神經網路訓練。



###### 3.7 必勝攻擊之訓練策略

​		[2]參考文獻提到了必勝攻擊 (Winning Attack) ，也是本研究的「迫著空間」其中一個特例，即若有「必勝走步可以執行時」，就立即執行必勝落子，會在走步產生時率先處理出這樣的走步，此方式亦可避免 *MCTs* 做多餘的搜尋運算，而之所以有這樣的「必勝攻擊」想法，關鍵在於 *MCTs* 會選擇 *UCT* 值最高的節點，表示被選擇的節點與其下之子樹之「平均價值」是當前較高的，但不一定是「絕對」價值最好的，以人類對弈認知上，所謂的必勝攻擊即為「絕對價值最好的」一步棋，但 *MCTs* 卻有很大的概率不會走「必勝攻擊」的走步，這不僅會延長棋局，更有可能被對手翻盤，因此當我們在產生走步時，會在「即將勝利的盤面」下，將合法的走步規則篩選成「必勝走步」才是合法的，就可以在產生走步階段得到我們要的結果，而文獻[2]中提及此內容，附圖如下：

![](https://i.imgur.com/DD3Nkag.png)


<center><font size=2>4</font></center>
<div style="page-break-after:always;"></div>

---


##### 肆、開發工具

###### 4.1 SDL (Simple DirectMedia Layer)

​		SDL是一套開放原始碼的「跨平台多媒體開發函式庫」，基於C語言所開發，其可控制圖像、聲音等功能，並可應用在開發三種不同作業系統 (Linux、MacOS和Window) 平台上的應用軟體，包括「雷神之鎚4」此款商業遊戲即透過SDL所開發，該款工具預計使用於本專題的 GUI介面上。

![](https://i.imgur.com/CqnWrsO.png)


###### 4.2 iTerm2

​    	iTerm2 為 MacOS作業系統上的一款終端機模擬工具，功能大致與Ubuntu的終端機大抵相同，可進行各式操作，例如透過Ranger瀏覽資料夾、用vim, emacs編輯文件，以git進行版本控制等操作。

![](https://i.imgur.com/nV8rkPZ.png)


###### 4.3 Vim

​		Vim為傳統文字編輯器之一，有「編輯器之神」之美稱，並以極為陡峭的學習曲線聞名，其愛好者稱其「眼到即手到」便可對其效率略知一二，仰賴鍵盤因此需搭配複雜多變的快捷鍵，可讓使用者聚焦於鍵盤之操作，故效率甚高。

![](https://i.imgur.com/pL6T6H6.png)


###### 4.4 Git

​		Git是非常有名的版本控制系統，以其分散式的版本控制形式深受人們愛戴，預計於本專題逐一擴展功能時使用，並善加運用Branch開發新功能，最終推上Github，善盡開源之義務。

![](https://i.imgur.com/VBwH8xs.png)


###### 4.5 GCC

​		GCC為C語言主要使用的編譯器，將以Vim配合Git撰寫專題程式碼，再以GCC編譯之，必要時會透過SSH連接至Ubuntu環境作業。

![](https://i.imgur.com/RpeIu6Q.png)


<center><font size=2>5</font></center>
<div style="page-break-after:always;"></div>

---

##### 伍、研究方法與步驟

***5.1  以 SDL製作跨平台之對弈介面，並新增程式端端口***

​		透過 SDL 製作對弈介面，並可選擇以人工、電腦方式對局，將此架構撰寫好後，便可從電腦端輸入對局計算之結果。下圖為本專題於 SDL之運算機制。

![](https://i.imgur.com/0sOn0r0.png)




​        起初先建造遊戲視窗，並且匯入圖片資訊於程式中，接著將屏幕畫面清除，同時偵測滑鼠按鍵狀況、鍵盤按鈕以及王將是否已死亡，再來先將背景、棋盤貼上屏幕，並顯示走步(藍色提示格子)，才印出棋子來，並偵測點擊為哪一個棋後新增深色提示，而在 *RenderPesent* 才將以上 *Render* 的所有畫面印出，再重複依照以上步驟去呈現畫面。



<center><font size=2>6</font></center>
<div style="page-break-after:always;"></div>

![](https://i.imgur.com/SOyClvn.png)



​        上圖為遊戲畫面，藍色格子表示深色棋子之走步範圍，紅色棋子代表升變過的棋種，棋盤外則為俘虜軍位於打入區內，操作以滑鼠、鍵盤進行，若任一方王將死亡便會停止，按 `a` 開啟新局。



***5.2  以 MCTs 撰寫 5五將棋 AI程式***

​		以常見的 MCTs 算法，在取得 **合法走步** 時 定義迫著空間再傳入 MCTs 中計算，能有效剪枝。

![](https://i.imgur.com/t8aoj26.png)


> 上圖呈現了一般MCTs與使用迫著空間之MCTs之不同處，最大的差異在於取得 **合法走步** 階段，就已先計算好 **迫著空間** 走步。

<center><font size=2>7</font></center>
<div style="page-break-after:always;"></div>

***5.3  以未使用或其他程式 與 MCTs(加入迫著空間) 對局 1000次***

​		已設計傳入演算法運算結果之接口，故可以將其與原先的 MCTs 或其他不同的5五將棋程式對弈，模擬 1000次後調查其勝率和平均下子時間，作為最終數據統計之結果。



###### 5.4 開發流程

![](https://i.imgur.com/guuYxTG.png)


​		在前期的規劃裡，則如上圖，橘色處為已完成、黃色處為正在進行，除了本研究所提及的「迫著空間」加速MCTs的方式外，在文獻探討的範疇中同時會探討AlphaZero以及近期由DeepMind所開發的MuZero，嘗試從中思考出有何能加速MCTs的算法。

​        目前在搜索程式部分，依據實際對弈結果，在產生「合法走步」階段對於迫著空間之定義有兩種看法：

1. 以**王將**為中心之 **龍馬** 之走步範圍內敵方 $\geq 2$
2. **中間至己方區塊**內敵方數 $\geq 2$

​     若在產生合法走步階段，先計算符合迫著空間定義之走步，使MCTs避免掉不必要的節點擴展、減少搜索層數，依此增加運算速度，再經過測試與修正，得到實驗結果(勝率 & 平均下子時間)。

​    在本階段中，基於SDL的介面之開發仍需加強，如 *Training data* 匯入、棋譜查詢、攻守交換、悔棋和特殊規則等項目需要實現，以提高操作、測試之便利性。





<center><font size=2>8</font></center>
<div style="page-break-after:always;"></div>

​        



![](https://i.imgur.com/lpTtD4s.png)



​		在後期的規劃中，如上圖粉底、紫底處，搜索程式可再與Deep Learning對接，另加上局勢評估函數以及DNN，則有望大大提升棋力，此外，殘局庫、開局庫的開發，與研發節省空間、運算快速的貯存方式，則更有利於對局表現的昇華。













<center><font size=2>9</font></center>

---

##### 柒、參考文獻

[1]  5 五將棋程式 Wonders 的設計與實作，國立臺灣師範大學資訊工程研究所碩士論文，張懷文，2014年。

[2] 利用 AlphaZero 框架實作與改良 MiniShogi 程式，國立臺灣師範大學資訊工程研究所碩士論文，陳品源，2010年。

[3] Wikipedia——5五將棋

https://zh.wikipedia.org/wiki/5%E4%BA%94%E5%B0%87%E6%A3%8B

[4] Wikipedia——將棋

https://zh.wikipedia.org/zh-tw/%E5%B0%86%E6%A3%8B_(%E6%97%A5%E6%9C%AC)

[5] A Survey of Monte Carlo Tree Search Methods，2012年

http://www.incompleteideas.net/609%20dropbox/other%20readings%20and%20resources/MCTS-survey.pdf

[6] Mastering Chess and Shogi by Self-Play with a General Reinforcement Learning Algorithm，2017年

https://arxiv.org/abs/1712.01815

[7] Mastering Atari, Go, Chess and Shogi by Planning with a Learned Model，2019年

https://arxiv.org/abs/1911.08265



<center><font size=2>10</font></center>