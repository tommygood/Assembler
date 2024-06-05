# SIC/XE Assembler
## 介紹
### 功能
- 這是一個由 C 語言開發的 One Pass SIC/XE Assembler
- 若 source program 沒有被偵測到 error，則會輸出對應的 object program，否則只會輸出所有的 error msg（包含哪一行發生 error 及詳細資訊），而不輸出 object program
### 特色
- 支援 program relocation，會在 object program 加入 Modification record
- 支援可以更改 program 的起始位址
### error case
1. redefined symbol
2. undefined symbol
3. invalid mnemonic
4. first executable instruction is missed
5. BASE Register is not defined, while base-relative is used
6. basic instruction before the program start
7. Symbol name conflict with mnemonic or operand
8. Integer Operand must be decimal in the WORD, RESW, RESB
9. RSUB can not have oeprand
### 資料結構
#### Hash
- hash function : mod（將字串的所有字元 ascii code 加總後，和 hash table length 相除取餘數得到 index）
- probing method : linear probing
- 利用 hash 去製作 table of opcode & symbol & forward reference
  - 因為在初始時，會先去 scan 一次 source program 去確認總共的行數，故可以確保 hash table 的 bucket 數量足夠，進而讓搜尋的時間複雜度維持在一定水準（In Avg Case, store and search is Θ(1)）

### 流程圖
<img src="https://hackmd.io/_uploads/rypH1g0N0.png"/>

### Demo
#### test program 1
- <a href = "https://github.com/tommygood/Assembler/blob/master/testprog3.S">source program</a> 
- Assembler output
  - ![image](https://github.com/tommygood/Assembler/assets/96759292/96cd9a14-58af-400a-922d-60bac106df76)
### 其他
- <a href="https://hackmd.io/@tommygood/SIC-XE-Assembler">note</a>
- <a href="https://hackmd.io/@tommygood/SIC-XE-Demo-Slide">demo slide</a>
## Prerequisite
- gcc : 可以在 Linux & Windows 上利用 gcc 編譯並執行。（於 Ubuntu 20.04 & Win10 + `gcc version 9.2.0` 測試）
## Usage
- `gcc main.c -o main -w`
- `main`
