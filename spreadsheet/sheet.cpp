#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

/*
    Метод SetCell
    Задаёт содержимое ячейки по индексу Position. 
    Если ячейка пуста, надо её создать. 
    Нужно задать ячейке текст методом Cell::Set(std::string);
*/
void Sheet::SetCell(Position pos, std::string text) {
    if(pos.IsValid()) {
        cells_.resize(std::max(pos.row + 1, int(std::size(cells_))));
        cells_[pos.row].resize(std::max(pos.col + 1, int(std::size(cells_[pos.row]))));

        if (!cells_[pos.row][pos.col]) {
            cells_[pos.row][pos.col] = std::make_unique<Cell>(*this);
        }
        cells_[pos.row][pos.col]->Set(std::move(text));
    }
    else {
        throw InvalidPositionException("from SetCell invalid cell position");
    }
}

/*
    Методы GetCell
    Константный и неконстантный геттеры, 
    которые возвращают указатель на ячейку, 
    расположенную по индексу pos. 
    Если ячейка пуста, возвращают nullptr;
*/
const CellInterface* Sheet::GetCell(Position pos) const {
    if (pos.IsValid()) {
        if (pos.row < int(std::size(cells_)) && pos.col < int(std::size(cells_[pos.row]))) {

            if (cells_[pos.row][pos.col].get()->GetText() == "") {
                return nullptr;

            }
            else {
                return cells_[pos.row][pos.col].get();
            }

        }
        else {
            return nullptr;
        }

    }
    else {
        throw InvalidPositionException("from GetCel invalid cell position");
    }
}
CellInterface* Sheet::GetCell(Position pos) {
    if (pos.IsValid()) {
        if (pos.row < int(std::size(cells_)) && pos.col < int(std::size(cells_[pos.row]))) {

            if (cells_[pos.row][pos.col].get()->GetText() == "") {
                return nullptr;
            }
            else {
                return cells_[pos.row][pos.col].get();
            }
        }
        else {
            return nullptr;
        }
    }
    else {
        throw InvalidPositionException("from GetCell invalid cell position");
    }
}

Cell* Sheet::Get_Cell(Position pos) {
    if (pos.IsValid()) {

        if (pos.row < int(std::size(cells_)) && pos.col < int(std::size(cells_[pos.row]))) {
            return cells_[pos.row][pos.col].get();

        }
        else {
            return nullptr;
        }

    }
    else {
        throw InvalidPositionException("from Get_Cell invalid cell position");
    }
}
const Cell* Sheet::Get_Cell(Position pos) const {
    const Cell* const_get_cell = Get_Cell(pos);
    return const_get_cell;
}

/*
    Метод ClearCell
    Очищает ячейку по индексу. 
    Последующий вызов GetCell() для этой ячейки вернёт nullptr.
*/
void Sheet::ClearCell(Position pos) {
    if (pos.IsValid()) {

        if (pos.row < int(std::size(cells_)) && pos.col < int(std::size(cells_[pos.row]))) {

            if (cells_[pos.row][pos.col]) {
                cells_[pos.row][pos.col]->Clear();

                if (!cells_[pos.row][pos.col]->IsReferenced()) {
                    cells_[pos.row][pos.col].reset();
                }
            }
        }

    }
    else {
        throw InvalidPositionException("from ClearCell invalid cell position");
    }
}
/*
    Метод GetPrintableSize
    Определяет размер минимальной печатной области
*/
Size Sheet::GetPrintableSize() const {
    Size printable_size;
    for (int row = 0; row < int(std::size(cells_)); ++row) {
        for (int col = (int(std::size(cells_[row])) - 1); col >= 0; --col) {

            if (cells_[row][col]) {

                if (cells_[row][col]->GetText().empty()) {
                    continue;

                }
                else {
                    printable_size.rows = std::max(printable_size.rows, row + 1);
                    printable_size.cols = std::max(printable_size.cols, col + 1);
                    break;
                }
            }
        }
    }
    return printable_size;
}

/*
    Метод PrintValues
    выводит значения ячеек — строки, числа или FormulaError, — как это определено в Cells::GetValue()
*/
void Sheet::PrintValues(std::ostream& output) const {
    for (int row = 0; row < GetPrintableSize().rows; ++row) {
        for (int col = 0; col < GetPrintableSize().cols; ++col) {

            if (col > 0) { 
                output << '\t'; 
            }

            if (col < int(std::size(cells_[row]))) {
                if (cells_[row][col]) {
                    std::visit([&output](const auto& obj) { output << obj; },
                        cells_[row][col]->GetValue());
                }
            }
        }

        output << '\n';
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for (int row = 0; row < GetPrintableSize().rows; ++row) {
        for (int col = 0; col < GetPrintableSize().cols; ++col) {

            if (col) { output << '\t'; }

            if (col < int(std::size(cells_[row]))) {
                if (cells_[row][col]) { output << cells_[row][col]->GetText(); }
            }
        }

        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}