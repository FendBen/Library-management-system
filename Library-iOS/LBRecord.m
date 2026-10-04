//
//  LBRecord.m
//
#import "LBRecord.h"

@implementation LBRecord

- (instancetype)initWithDictionary:(NSDictionary *)d {
    self = [super init];
    if (self) {
        _recordId = d[@"id"] ?: @"";
        _bookId = d[@"bookId"] ?: @"";
        _readerId = d[@"readerId"] ?: @"";
        _borrowDate = d[@"borrowDate"] ?: @"";
        _dueDate = d[@"dueDate"] ?: @"";
        _returnDate = d[@"returnDate"] ?: @"";
    }
    return self;
}

- (NSDictionary *)toDictionary {
    return @{
        @"id": self.recordId ?: @"",
        @"bookId": self.bookId ?: @"",
        @"readerId": self.readerId ?: @"",
        @"borrowDate": self.borrowDate ?: @"",
        @"dueDate": self.dueDate ?: @"",
        @"returnDate": self.returnDate ?: @"",
    };
}

- (BOOL)isReturned {
    return self.returnDate.length > 0;
}

@end
