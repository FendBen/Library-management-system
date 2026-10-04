//
//  LBBook.h — 图书模型
//
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface LBBook : NSObject

@property (nonatomic, copy) NSString *bookId;      // 如 B001
@property (nonatomic, copy) NSString *title;
@property (nonatomic, copy) NSString *author;
@property (nonatomic, copy) NSString *isbn;
@property (nonatomic, assign) NSInteger totalCopies;
@property (nonatomic, assign) NSInteger availableCopies;

- (instancetype)initWithDictionary:(NSDictionary *)d;
- (NSDictionary *)toDictionary;

@end

NS_ASSUME_NONNULL_END
