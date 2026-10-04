//
//  LBBook.m
//
#import "LBBook.h"

@implementation LBBook

- (instancetype)initWithDictionary:(NSDictionary *)d {
    self = [super init];
    if (self) {
        _bookId = d[@"id"] ?: @"";
        _title = d[@"title"] ?: @"";
        _author = d[@"author"] ?: @"";
        _isbn = d[@"isbn"] ?: @"";
        _totalCopies = [d[@"totalCopies"] integerValue];
        _availableCopies = [d[@"availableCopies"] integerValue];
    }
    return self;
}

- (NSDictionary *)toDictionary {
    return @{
        @"id": self.bookId ?: @"",
        @"title": self.title ?: @"",
        @"author": self.author ?: @"",
        @"isbn": self.isbn ?: @"",
        @"totalCopies": @(self.totalCopies),
        @"availableCopies": @(self.availableCopies),
    };
}

@end
