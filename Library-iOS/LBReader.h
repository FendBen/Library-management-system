//
//  LBReader.h — 读者模型
//
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface LBReader : NSObject

@property (nonatomic, copy) NSString *readerId;    // 如 R001
@property (nonatomic, copy) NSString *name;
@property (nonatomic, copy) NSString *phone;

- (instancetype)initWithDictionary:(NSDictionary *)d;
- (NSDictionary *)toDictionary;

@end

NS_ASSUME_NONNULL_END
