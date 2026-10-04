//
//  LBRecord.h — 借阅记录模型
//
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface LBRecord : NSObject

@property (nonatomic, copy) NSString *recordId;    // 如 BR001
@property (nonatomic, copy) NSString *bookId;
@property (nonatomic, copy) NSString *readerId;
@property (nonatomic, copy) NSString *borrowDate;  // YYYY-MM-DD
@property (nonatomic, copy) NSString *dueDate;     // YYYY-MM-DD
@property (nonatomic, copy) NSString *returnDate;  // 空串 = 未归还

- (instancetype)initWithDictionary:(NSDictionary *)d;
- (NSDictionary *)toDictionary;
- (BOOL)isReturned;

@end

NS_ASSUME_NONNULL_END
