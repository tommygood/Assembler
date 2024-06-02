# SIC/XE Assembler
## 介紹
### 功能
- 這是一個由 C 語言開發的 One Pass SIC/XE Assembler
- 若 source program 沒有被偵測到 error，則會輸出對應的 object program，否則只會輸出所有的 error msg（包含哪一行發生 error 及詳細資訊），而不輸出 object program
### 特色
- 支援 program relocation，會在 object program 加入 Modification record
- 支援可以更改 program 的起始位址
### error case
1. redefined symbol :
2. undefined symbol :
3. invalid mnemonic : 
4. first executable instruction is missed :
5. BASE Register is not defined, while base-relative is used :
6. basic instruction before the program start :
### 其他
- 環境：可以在 Linux & Windows 上利用 gcc 編譯並執行。（於 Ubuntu 20.04 & Win10 + `gcc version 9.2.0` 測試）
- <a href = "https://hackmd.io/@tommygood/SIC-XE-Assembler">筆記</a>
## 資料結構
### Hash
- hash function : mod（將字串的所有字元 ascii code 加總後，和 hash table length 相除取餘數得到 index）
- 利用 hash 去製作 table of opcode & symbol & forward reference
  - 因為在初始時，會先去 scan 一次 source program 去確認總共的行數，故可以確保 hash table 的 bucket 數量足夠，進而讓搜尋的時間複雜度維持在一定水準（In Avg Case, store and search is Θ(1)）
  - 這邊的 forward reference table 不用使用 linked list 的原因：因 hash value 是以 `symbol forward_reference_line_number_1 forward_reference_line_number_2` 的格式去存值。<br/>
  但在做 hash function 找 index 時，會以 split 去切割空白，並只取用 symbol 作為要做 hash function 的對象，所以得到的 index 也會是以該 symbol 計算而來的。
## 流程圖
## Demo
### test program 1
- <a href = "https://github.com/tommygood/Assembler/blob/master/testprog3.S">source program</a> 
- Assembler output ![image](https://github.com/tommygood/Assembler/assets/96759292/2c32bc4e-22f8-472b-ab30-7d07e1f4dc9b)
