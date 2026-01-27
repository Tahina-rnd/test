import {
  Controller,
  Get,
  Post,
  Body,
  Patch,
  Param,
  Delete,
  Query,
  UseGuards,
} from '@nestjs/common';
import { AttendanceService } from './attendance.service';
import { CreateAttendanceDto } from './dto/create-attendance.dto';
import { UpdateAttendanceDto } from './dto/update-attendance.dto';
import { JwtAuthGuard } from '@common/guards/jwt-auth.guard';
import { RolesGuard } from '@common/guards/roles.guard';
import { Roles } from '@decorators/roles.decorator';
import { UserRole } from '@enums/user.enum';

@Controller('attendance')
@UseGuards(JwtAuthGuard, RolesGuard)
export class AttendanceController {
  constructor(private readonly attendanceService: AttendanceService) {}

  /**
   * Create a new attendance record.
   *
   * @param createAttendanceDto - The attendance data to create.
   *
   * @returns The created attendance record.
   */
  @Post()
  @Roles(UserRole.ADMIN)
  create(@Body() createAttendanceDto: CreateAttendanceDto) {
    return this.attendanceService.create(createAttendanceDto);
  }

  /**
   * Get all attendance records.
   *
   * @returns An array of attendance records.
   */
  @Get()
  @Roles(UserRole.ADMIN)
  findAll() {
    return this.attendanceService.findAll();
  }

  /**
   * Get an attendance record by id.
   *
   * @param id - The id of the attendance record to retrieve.
   *
   * @returns The attendance record.
   */
  @Get(':id')
  @Roles(UserRole.ADMIN, UserRole.STUDENT)
  findOne(@Param('id') id: string) {
    return this.attendanceService.findOne(id);
  }

  /**
   * Get attendance records by student id.
   *
   * @param studentId - The student id to get attendance records for.
   *
   * @returns An array of attendance records.
   */
  @Get('student/:studentId')
  @Roles(UserRole.ADMIN, UserRole.STUDENT)
  findByStudentId(@Param('studentId') studentId: string) {
    return this.attendanceService.findByStudentId(studentId);
  }

  /**
   * Get attendance records by subject code.
   *
   * @param subjectCode - The subject code to get attendance records for.
   *
   * @returns An array of attendance records.
   */
  @Get('subject/:subjectCode')
  @Roles(UserRole.ADMIN)
  findBySubject(@Param('subjectCode') subjectCode: string) {
    return this.attendanceService.findBySubject(subjectCode);
  }

  /**
   * Get attendance records by date range.
   *
   * @param startDate - The start date.
   * @param endDate - The end date.
   *
   * @returns An array of attendance records.
   */
  @Get('daterange')
  @Roles(UserRole.ADMIN)
  findByDateRange(
    @Query('startDate') startDate: string,
    @Query('endDate') endDate: string,
  ) {
    return this.attendanceService.findByDateRange(
      new Date(startDate),
      new Date(endDate),
    );
  }

  /**
   * Calculate attendance percentage for a student.
   *
   * @param studentId - The student id to calculate attendance percentage for.
   *
   * @returns The attendance percentage.
   */
  @Get('percentage/:studentId')
  @Roles(UserRole.ADMIN, UserRole.STUDENT)
  calculateAttendancePercentage(@Param('studentId') studentId: string) {
    return this.attendanceService.calculateAttendancePercentage(studentId);
  }

  /**
   * Get attendance statistics for a subject.
   *
   * @param subjectCode - The subject code to get statistics for.
   *
   * @returns Attendance statistics.
   */
  @Get('stats/:subjectCode')
  @Roles(UserRole.ADMIN)
  getSubjectAttendanceStats(@Param('subjectCode') subjectCode: string) {
    return this.attendanceService.getSubjectAttendanceStats(subjectCode);
  }

  /**
   * Update an attendance record.
   *
   * @param id - The id of the attendance record to update.
   * @param updateAttendanceDto - The attendance data to update.
   *
   * @returns The updated attendance record.
   */
  @Patch(':id')
  @Roles(UserRole.ADMIN)
  update(
    @Param('id') id: string,
    @Body() updateAttendanceDto: UpdateAttendanceDto,
  ) {
    return this.attendanceService.update(id, updateAttendanceDto);
  }

  /**
   * Delete an attendance record.
   *
   * @param id - The id of the attendance record to delete.
   */
  @Delete(':id')
  @Roles(UserRole.ADMIN)
  remove(@Param('id') id: string) {
    return this.attendanceService.remove(id);
  }
}
