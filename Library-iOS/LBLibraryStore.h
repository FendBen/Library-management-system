//
//  LBLibraryStore.h — 数据层 + 业务规则（单例）
//
#import <Foundation/Foundation.h>
#import "LBBook.h"
#import "LBReader.h"
#import "LBRecord.h"

NS_ASSUME_NONNULL_BEGIN

@interface LBLibraryStore : NSObject

@property (nonatomic, readonly) NSMutableArray<LBBook *> *books;
@property (nonatomic, readonly) NSMutableArray<LBReader *> *readers;
@property (nonatomic, readonly) NSMutableArray<LBRecord *> *records;

+ (instancetype)sharedStore;

/// 从 Documents/library.json 加载；无文件则初始化空库
- (void)load;

/// 图书
- (LBBook *)addBookTitle:(NSString *)title author:(NSString *)author
                    isbn:(NSString *)isbn copies:(NSInteger)copies
                   error:(NSString *_Nullable *_Nullable)err;
- (BOOL)removeBook:(NSString *)bookId error:(NSString *_Nullable *_Nullable)err;
- (BOOL)updateBook:(NSString *)bookId title:(NSString *)title author:(NSString *)author
              isbn:(NSString *)isbn copies:(NSInteger)copies
             error:(NSString *_Nullable *_Nullable)err;
- (NSArray<LBBook *> *)searchBooks:(NSString *)keyword;
- (nullable LBBook *)bookById:(NSString *)bookId;

/// 读者
- (LBReader *)addReaderName:(NSString *)name phone:(NSString *)phone
                      error:(NSString *_Nullable *_Nullable)err;
- (BOOL)removeReader:(NSString *)readerId error:(NSString *_Nullable *_Nullable)err;
- (nullable LBReader *)readerById:(NSString *)readerId;

/// 借还
- (BOOL)borrowBook:(NSString *)bookId byReader:(NSString *)readerId
              days:(NSInteger)days error:(NSString *_Nullable *_Nullable)err;
- (BOOL)returnBook:(NSString *)bookId byReader:(NSString *)readerId
             error:(NSString *_Nullable *_Nullable)err;
- (NSArray<LBRecord *> *)activeRecords;
- (BOOL)recordIsOverdue:(LBRecord *)record;

/// 统计
- (NSDictionary *)stats;

/// 工具
+ (NSString *)todayString;
+ (NSString *)dateByAddingDays:(NSInteger)days toDate:(NSString *)date;

@end

NS_ASSUME_NONNULL_END
