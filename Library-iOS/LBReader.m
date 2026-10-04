//
//  LBReader.m
//
#import "LBReader.h"

@implementation LBReader

- (instancetype)initWithDictionary:(NSDictionary *)d {
    self = [super init];
    if (self) {
        _readerId = d[@"id"] ?: @"";
        _name = d[@"name"] ?: @"";
        _phone = d[@"phone"] ?: @"";
    }
    return self;
}

- (NSDictionary *)toDictionary {
    return @{
        @"id": self.readerId ?: @"",
        @"name": self.name ?: @"",
        @"phone": self.phone ?: @"",
    };
}

@end
