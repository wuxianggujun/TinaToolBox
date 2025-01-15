#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <variant>
#include <arrow/compute/api.h>
#include <arrow/result.h>
#include <parquet/arrow/writer.h>
#include <arrow/api.h>
#include <arrow/table.h>
#include <thread>
#include <mutex>
#include <ThreadPool.hpp>


namespace TinaToolBox {
    
    class DataFrame {
    public:
        DataFrame() = default;
        explicit DataFrame(std::shared_ptr<arrow::Table> table);

        static DataFrame fromExcel(const std::string &filePath);
        
        // 基本操作
        [[nodiscard]] size_t rowCount() const { return table_ ? table_->num_rows() : 0; }
        [[nodiscard]] size_t columnCount() const { return table_ ? table_->num_columns() : 0; }
        [[nodiscard]] std::vector<std::string> getColumnNames() const;
        
        // 数据访问
        [[nodiscard]] std::shared_ptr<arrow::ChunkedArray> getColumn(const std::string &name) const;
        [[nodiscard]] arrow::Result<std::shared_ptr<arrow::RecordBatch>> getRow(int64_t index) const;

        // arrow::Result<DataFrame> filter(const std::string &column,
        //                                const std::shared_ptr<arrow::Scalar> &value) const;
        // 数据操作
        arrow::Result<DataFrame> filter(
            const std::string& column,
            const std::shared_ptr<arrow::Scalar>& value,
            const std::string& comparison_operator = "equal"
        ) const;

        arrow::Result<DataFrame> sort(
            const std::string& column, 
            bool ascending = true
        ) const;
        
        // 类型安全的值获取
        template<typename T>
        arrow::Result<T> getValue(int64_t row, const std::string &column) const;
        
        [[nodiscard]] bool toSaveExcel(const std::string &filePath, bool forceOverwrite = false) const;
        
        // Arrow Table 访问器
        [[nodiscard]] std::shared_ptr<arrow::Table> table() const { return table_; }
        [[nodiscard]] std::shared_ptr<arrow::Schema> schema() const { return table_ ? table_->schema() : nullptr; }
    private:
        std::shared_ptr<arrow::Table> table_;
        static ThreadPool& getThreadPool() {
            static ThreadPool pool;  // 单例线程池
            return pool;
        }
        // 将 appendBatch 改为非静态成员函数
       static void appendBatch(std::shared_ptr<arrow::ArrayBuilder>& builder,
                        const std::vector<std::variant<std::string, double, int64_t>>& batch,
                        const std::shared_ptr<arrow::DataType>& type);
    };
}
